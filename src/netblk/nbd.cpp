#if defined(HAVE_LINUX_NBD_H)
#include<list>
#include<cerrno>
#include<cstring>
#include<csignal>
#include<fcntl.h>
#include<endian.h>
#include<semaphore>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<sys/prctl.h>
#include<sys/socket.h>
#include<sys/sysmacros.h>
#include<linux/nbd.h>
#include"fs-utils.h"
#include"internal.h"
#include"worker.h"
#include"modules.h"
#include"error.h"
#include"log.h"

class netblk_device_nbd;
class netblk_implement_nbd:
	public netblk_implement,
	public std::enable_shared_from_this<netblk_implement_nbd>
{
	public:
		~netblk_implement_nbd();
		inline std::string get_name()const override{return "nbd";}
		void try_enable()override;
		bool is_supported()const override;
		void init()override;
		void stop()override;
		void run()override;
		std::shared_ptr<netblk_device>create(const std::shared_ptr<netblk_backend>&backend)override;
		std::list<std::shared_ptr<netblk_device_nbd>>devices{};
};

class netblk_nbd_context{
	public:
		std::shared_ptr<netblk_device_nbd>dev;
		nbd_request req{};
		nbd_reply reply{};
		std::string buffer{};
		bool reply_sent=false;
		void process();
		void reply_error(int err);
		void fill_reply(int err=0);
};

class netblk_device_nbd:
	public netblk_device,
	public std::enable_shared_from_this<netblk_device_nbd>
{
	public:
		~netblk_device_nbd();
		std::string get_path()const override;
		void init();
		void destroy()override;
		void thread_int_main();
		void thread_read_main();
		void read_nbd();
		void request_stop();
		std::thread thread_int{};
		std::thread thread_read{};
		std::mutex write_lock{};
		std::counting_semaphore<2>sem_ready{0};
		std::shared_ptr<netblk_backend>get_backend()override{return backend;}
		std::shared_ptr<netblk_implement_nbd>get_impl()const;
		std::shared_ptr<netblk_backend>backend=nullptr;
		std::weak_ptr<netblk_implement_nbd>impl;
		std::atomic<bool>running=false;
		size_t size=0;
		int nbd_fd=-1;
		int nbd_id=-1;
		int sock_fd[2]{-1,-1};
};

static int nbd_find_free(){
	int max=16;
	try{
		auto val=fs_simple_read("/sys/module/nbd/parameters/nbds_max");
		max=std::stoi(val);
	}catch(std::exception&exc){
		log_exception(exc,"failed to read nbd max");
	}
	struct stat st{};
	for(int i=0;i<max;i++){
		auto path=std::format("/dev/nbd{}",i);
		if(stat(path.c_str(),&st)<0)continue;
		if(!S_ISBLK(st.st_mode))continue;
		if(major(st.st_rdev)!=43)continue;
		if(minor(st.st_rdev)!=i)continue;
		auto sz_path=std::format("/sys/class/block/nbd{}/size",i);
		auto sz=fs_simple_read(sz_path);
		if(sz!="0")continue;
		return i;
	}
	log_warning("no free nbd device found");
	return -1;
}

netblk_implement_nbd::~netblk_implement_nbd(){
	stop();
}

netblk_device_nbd::~netblk_device_nbd(){
	destroy();
}

std::shared_ptr<netblk_implement_nbd>netblk_device_nbd::get_impl()const{
	auto impl=this->impl.lock();
	if(!impl)throw RuntimeError("implement is null");
	return impl;
}

void netblk_implement_nbd::try_enable(){
	module_load("nbd");
	module_load("unix");
}

bool netblk_implement_nbd::is_supported()const{
	return 
		fs_exists("/sys/module/nbd")&&
		fs_exists("/proc/net/unix")&&
		nbd_find_free()>=0;
}

std::shared_ptr<netblk_implement>netblk_implement_create_nbd(const Json::Value&opts){
	auto impl=std::make_shared<netblk_implement_nbd>();
	return impl;
}

void netblk_implement_nbd::stop(){
	while(!devices.empty()){
		auto dev=devices.front();
		devices.pop_front();
		dev->destroy();
	}
}

void netblk_implement_nbd::run(){
	if(!running)return;
	bool any_running=false;
	for(auto&dev:devices){
		if(!dev->running)continue;
		any_running=true;
	}
	if(!any_running){
		log_info("no any devices running");
		running=false;
		kill(getpid(),SIGUSR1);
	}else sleep(1);
}

void netblk_implement_nbd::init(){
	running=true;
}

void netblk_device_nbd::destroy(){
	running=false;
	if(nbd_fd>=0){
		ioctl(nbd_fd,NBD_CLEAR_QUE);
		ioctl(nbd_fd,NBD_DISCONNECT);
		ioctl(nbd_fd,NBD_CLEAR_SOCK);
		close(nbd_fd);
		nbd_fd=-1;
	}
	if(sock_fd[0]>=0){
		close(sock_fd[0]);
		sock_fd[0]=-1;
	}
	if(sock_fd[1]>=0){
		close(sock_fd[1]);
		sock_fd[1]=-1;
	}
	if(thread_int.joinable())
		thread_int.join();
	if(thread_read.joinable())
		thread_read.join();
	if(!impl.expired())try{
		auto impl=get_impl();
		for(auto it=impl->devices.begin();it!=impl->devices.end();it++){
			if(it->get()!=this)continue;
			impl->devices.erase(it);
			break;
		}
	}catch(...){}
}

void netblk_device_nbd::request_stop(){
	running=false;
	kill(getpid(),SIGUSR1);
}

void netblk_device_nbd::thread_int_main(){
	prctl(PR_SET_NAME,"netblk-nbd-int");
	log_info("nbd {} interrupt thread started",gettid());
	sem_ready.release();
	auto ret=ioctl(nbd_fd,NBD_DO_IT);
	if(ret<0)log_error("failed to process nbd: {}",strerror(errno));
	ioctl(nbd_fd,NBD_CLEAR_QUE);
	ioctl(nbd_fd,NBD_DISCONNECT);
	ioctl(nbd_fd,NBD_CLEAR_SOCK);
	log_info("nbd {} interrupt thread exit",gettid());
	request_stop();
}

void netblk_nbd_context::fill_reply(int err){
	memset(&reply,0,sizeof(reply));
	if(err<0)err=-err;
	reply.magic=htobe32(NBD_REPLY_MAGIC);
	reply.error=htobe32(err);
	memcpy(&reply.handle,&req.handle,sizeof(req.handle));
}

void netblk_nbd_context::process(){
	ssize_t ret=0;
	auto type=be32toh(req.type);
	auto offset=be64toh(req.from);
	auto len=be32toh(req.len);
	if(buffer.size()!=len)
		buffer.resize(len);
	void*buf=buffer.data();
	auto size=buffer.size();
	switch(type){
		case NBD_CMD_DISC:dev->request_stop();break;
		case NBD_CMD_READ:ret=dev->backend->do_read(offset,buf,size);break;
		case NBD_CMD_WRITE:ret=dev->backend->do_write(offset,buf,size);break;
		default:log_warning("nbd invalid command {}",type);ret=ENOTSUP;break;
	}
	if(ret<0)throw ErrnoErrorWith(-ret,"nbd command failed");
	fill_reply();
	std::lock_guard<std::mutex>lock(dev->write_lock);
	full_write(dev->sock_fd[1],&reply,sizeof(reply));
	reply_sent=true;
	if(type==NBD_CMD_READ)full_write(dev->sock_fd[1],buf,size);
}

void netblk_nbd_context::reply_error(int err){
	if(reply_sent)return;
	if(err==0)err=EIO;
	reply_sent=true;
	fill_reply(err);
	std::lock_guard<std::mutex>lock(dev->write_lock);
	full_write(dev->sock_fd[1],&reply,sizeof(reply));
}

void netblk_device_nbd::read_nbd(){
	auto ctx=std::make_shared<netblk_nbd_context>();
	ctx->dev=shared_from_this();
	ssize_t ret=read(sock_fd[1],&ctx->req,sizeof(ctx->req));
	if(ret<0){
		if(errno==EAGAIN)return;
		if(errno==EINTR)return;
		throw ErrnoError("failed to read nbd");
	}
	if(ret==0)throw RuntimeError("nbd read reached EOF");
	if(ret!=sizeof(nbd_request))throw RuntimeError("nbd read invalid size {}",ret);
	if(ctx->req.magic!=htobe32(NBD_REQUEST_MAGIC))
		throw RuntimeError("nbd read invalid magic");
	if(ctx->req.type==htobe32(NBD_CMD_WRITE)){
		ctx->buffer.resize(be32toh(ctx->req.len));
		ret=full_read(sock_fd[1],ctx->buffer.data(),ctx->buffer.size());
		if(ret!=ctx->buffer.size())
			throw RuntimeError("nbd read invalid size {}",ret);
	}
	worker_add([ctx]{
		int ret=EIO;
		try{
			ctx->process();
		}catch(ErrnoErrorImpl&exc){
			log_exception(exc,"nbd process error");
			ret=exc.err;
		}catch(std::exception&exc){
			log_exception(exc,"nbd process error");
		}
		ctx->reply_error(ret);
	});
}

void netblk_device_nbd::thread_read_main(){
	prctl(PR_SET_NAME,"netblk-nbd-read");
	log_info("nbd {} read thread started",gettid());
	sem_ready.release();
	try{
		while(running)read_nbd();
	}catch(std::exception&exc){
		log_exception(exc,"nbd read thread error");
	}
	log_info("nbd {} read thread exit",gettid());
	request_stop();
}

void netblk_device_nbd::init(){
	int ret;
	size=backend->get_size();
	nbd_id=nbd_find_free();
	if(nbd_id<0)throw RuntimeError("no free nbd device");
	nbd_fd=open(get_path().c_str(),O_RDWR|O_CLOEXEC);
	if(nbd_fd<0)throw ErrnoError("failed to open {}",get_path());
	ret=socketpair(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0,sock_fd);
	if(ret<0)throw ErrnoError("failed to create socket");
	if(sock_fd[0]<0||sock_fd[1]<0)
		throw RuntimeError("invalid socket pair");
	#ifdef NBD_SET_FLAGS
	int flags=0;
	#ifdef NBD_FLAG_READ_ONLY
	if(!backend->can_access(S_IWOTH))
		flags|=NBD_FLAG_READ_ONLY;
	#endif
	ioctl(nbd_fd,NBD_SET_FLAGS,flags);
	#endif
	ioctl(nbd_fd,NBD_CLEAR_SOCK);
	xioctl(nbd_fd,NBD_SET_SIZE,size);
	xioctl(nbd_fd,NBD_SET_SOCK,sock_fd[0]);
	running=true;
	thread_read=std::thread(&netblk_device_nbd::thread_read_main,this);
	thread_int=std::thread(&netblk_device_nbd::thread_int_main,this);
	backend->prefetch(0,0x100000);
	sem_ready.acquire();
	setup_block();
}

std::string netblk_device_nbd::get_path()const{
	if(nbd_id<0)return "";
	return std::format("/dev/nbd{}",nbd_id);
}

std::shared_ptr<netblk_device>netblk_implement_nbd::create(
	const std::shared_ptr<netblk_backend>&backend
){
	auto dev=std::make_shared<netblk_device_nbd>();
	dev->impl=shared_from_this();
	dev->backend=backend;
	dev->init();
	devices.push_back(dev);
	return dev;
}
#endif
