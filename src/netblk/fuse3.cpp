#ifdef USE_LIB_FUSE3
#define FUSE_USE_VERSION 35
#include<list>
#include<atomic>
#include<thread>
#include<cerrno>
#include<cstring>
#include<csignal>
#include<semaphore>
#include<fuse3/fuse.h>
#include<fuse3/fuse_lowlevel.h>
#include<sys/prctl.h>
#include"path-utils.h"
#include"fs-utils.h"
#include"internal.h"
#include"cleanup.h"
#include"modules.h"
#include"error.h"
#include"loop.h"
#include"log.h"

class netblk_device_fuse3;
class netblk_implement_fuse3:
	public netblk_implement,
	public std::enable_shared_from_this<netblk_implement_fuse3>
{
	public:
		~netblk_implement_fuse3();
		inline std::string get_name()const override{return "fuse3";}
		void try_enable()override;
		bool is_supported()const override;
		void init()override;
		void thread_main();
		void stop()override;
		void run()override;
		std::shared_ptr<netblk_device>create(const std::shared_ptr<netblk_backend>&backend)override;
		std::thread thread{};
		std::string mountpoint{};
		fuse_session*se=nullptr;
		std::list<std::shared_ptr<netblk_device_fuse3>>devices{};
		std::atomic<int>devid=0;
		std::mutex mutex{};
		std::binary_semaphore sem_ready{0};
		pid_t thread_id=0;
};

class netblk_device_fuse3:
	public netblk_device,
	public std::enable_shared_from_this<netblk_device_fuse3>
{
	public:
		~netblk_device_fuse3();
		std::string get_path()const override;
		std::string get_fuse_path()const;
		void init();
		void destroy()override;
		void setup_loop();
		void detach_loop();
		std::shared_ptr<netblk_backend>get_backend()override{return backend;}
		std::shared_ptr<netblk_implement_fuse3>get_impl()const;
		std::shared_ptr<netblk_backend>backend=nullptr;
		std::weak_ptr<netblk_implement_fuse3>impl;
		std::string fuse3_filename{};
		int loop_id=-1;
};

netblk_implement_fuse3::~netblk_implement_fuse3(){
	stop();
}

netblk_device_fuse3::~netblk_device_fuse3(){
	destroy();
}

std::shared_ptr<netblk_implement_fuse3>netblk_device_fuse3::get_impl()const{
	auto impl=this->impl.lock();
	if(!impl)throw RuntimeError("implement is null");
	return impl;
}

void netblk_implement_fuse3::try_enable(){
	module_load("fuse");
	module_load("loop");
}

bool netblk_implement_fuse3::is_supported()const{
	return fs_exists("/dev/fuse")&&fs_exists("/dev/loop-control");
}

std::shared_ptr<netblk_implement>netblk_implement_create_fuse3(const Json::Value&opts){
	auto impl=std::make_shared<netblk_implement_fuse3>();
	impl->mountpoint="/tmp/netblk";
	if(opts.isMember("mountpoint"))
		impl->mountpoint=opts["mountpoint"].asString();
	if(!fs_exists(impl->mountpoint)){
		auto ret=mkdir(impl->mountpoint.c_str(),0600);
		if(ret!=0&&errno!=EEXIST)
			throw ErrnoError("failed to create fuse3 mount point {}",impl->mountpoint);
		log_info("fuse3 mount point {} created",impl->mountpoint);
	}
	return impl;
}

struct netblk_fuse3_context{
	std::weak_ptr<netblk_implement_fuse3>dev;
};

static std::shared_ptr<netblk_implement_fuse3>fuse3_get_impl(){
	auto ctx=fuse_get_context();
	if(!ctx)return nullptr;
	auto pctx=(netblk_fuse3_context*)ctx->private_data;
	if(!pctx)return nullptr;
	return pctx->dev.lock();
}

static int netblk_getattr(
	const char*path,struct stat*stbuf,fuse_file_info*
){
	auto impl=fuse3_get_impl();
	if(!impl)return -ENOTCONN;
	memset(stbuf,0,sizeof(struct stat));
	std::string cpath=path;
	if(cpath=="/"){
		stbuf->st_mode=S_IFDIR|0555;
		stbuf->st_nlink=2;
		return 0;
	}
	if(cpath.length()>0&&cpath[0]=='/')
		cpath.erase(0,1);
	std::lock_guard<std::mutex>lock(impl->mutex);
	for(auto&dev:impl->devices){
		if(dev->fuse3_filename!=cpath)continue;
		if(!dev->backend)continue;
		stbuf->st_size=dev->backend->get_size();
		stbuf->st_mode=S_IFREG|0600;
		stbuf->st_nlink=1;
		return 0;
	}
	return -ENOENT;
}

static int netblk_readdir(
	const char*path,void*buf,fuse_fill_dir_t filler,
	off_t,fuse_file_info*,fuse_readdir_flags
){
	auto impl=fuse3_get_impl();
	if(!impl)return -ENOTCONN;
	if(strcmp(path,"/")!=0)return -ENOENT;
	filler(buf,".",nullptr,0,FUSE_FILL_DIR_PLUS);
	filler(buf,"..",nullptr,0,FUSE_FILL_DIR_PLUS);
	std::lock_guard<std::mutex>lock(impl->mutex);
	for(auto&dev:impl->devices){
		if(!dev->backend)continue;
		auto name=dev->fuse3_filename;
		if(name.empty())continue;
		filler(buf,name.c_str(),nullptr,0,FUSE_FILL_DIR_PLUS);
	}
	return 0;
}

static int netblk_open(const char*path,fuse_file_info*fi){
	auto impl=fuse3_get_impl();
	if(!impl)return -ENOTCONN;
	std::string cpath=path;
	if(cpath.length()>0&&cpath[0]=='/')
		cpath.erase(0,1);
	std::lock_guard<std::mutex>lock(impl->mutex);
	for(auto&dev:impl->devices){
		if(dev->fuse3_filename!=cpath)continue;
		if(!dev->backend)continue;
		return 0;
	}
	return -ENOENT;
}

static std::shared_ptr<netblk_backend>find_backend(
	const std::string&path
){
	auto impl=fuse3_get_impl();
	if(!impl)return nullptr;
	std::lock_guard<std::mutex>lock(impl->mutex);
	for(auto&dev:impl->devices){
		if(dev->fuse3_filename!=path)continue;
		if(!dev->backend)continue;
		return dev->backend;
	}
	return nullptr;
}

static int netblk_read(
	const char*path,char*buf,size_t size,
	off_t offset,fuse_file_info*
){
	auto impl=fuse3_get_impl();
	if(!impl)return -ENOTCONN;
	std::string cpath=path;
	if(cpath.length()>0&&cpath[0]=='/')
		cpath.erase(0,1);
	auto backend=find_backend(cpath);
	if(!backend)return -ENOENT;
	return backend->do_read(offset,buf,size);
}

static int netblk_write(
	const char*path,const char*buf,size_t size,
	off_t offset,fuse_file_info*
){
	auto impl=fuse3_get_impl();
	if(!impl)return -ENOTCONN;
	std::string cpath=path;
	if(cpath.length()>0&&cpath[0]=='/')
		cpath.erase(0,1);
	auto backend=find_backend(cpath);
	if(!backend)return -ENOENT;
	return backend->do_write(offset,buf,size);
}

static void*netblk_init(
	fuse_conn_info*conn,
	fuse_config*
){
	void*ret=nullptr;
	auto ctx=fuse_get_context();
	if(ctx)ret=ctx->private_data;
	if(conn)conn->max_read=67108864;
	return ret;
}

static const fuse_operations netblk_oper{
	.getattr = netblk_getattr,
	.open    = netblk_open,
	.read    = netblk_read,
	.write   = netblk_write,
	.readdir = netblk_readdir,
	.init    = netblk_init,
};

void netblk_implement_fuse3::thread_main(){
	int ret;
	bool ready=false;
	netblk_fuse3_context ctx{};
	static const char*argv[]={"netblk","-o","ro,max_read=67108864",nullptr};
	thread_id=gettid();
	try{
		ctx.dev=weak_from_this();
		prctl(PR_SET_NAME,"netblk-fuse3");
		log_info("fuse3 thread {} started",gettid());
		fuse_args args=FUSE_ARGS_INIT(3,(char**)argv);
		cleanup_func args_cleanup(std::bind(fuse_opt_free_args,&args));
		auto fuse=fuse_new(&args,&netblk_oper,sizeof(netblk_oper),&ctx);
		if(!fuse)throw RuntimeError("failed to create fuse3");
		cleanup_func fuse3_cleanup(std::bind(&fuse_destroy,fuse));
		if(!(se=fuse_get_session(fuse)))
			throw RuntimeError("failed to get fuse3 session");
		if(fuse_mount(fuse,mountpoint.c_str())<0)
			throw ErrnoError("failed to mount fuse3 at {}",mountpoint);
		cleanup_func fuse3_umount(std::bind(&fuse_unmount,fuse));
		sem_ready.release();
		ready=true;
		if((ret=fuse_loop(fuse))<0)
			throw ErrnoErrorWith(ret,"fuse3 loop failed");
		log_info("fuse3 thread {} stopped",gettid());
	}catch(std::exception&exc){
		log_exception(exc,"fuse3 thread error");
		if(!ready)sem_ready.release();
	}
	se=nullptr;
}

void netblk_implement_fuse3::stop(){
	try{
		std::lock_guard<std::mutex>lock(mutex);
		for(auto&dev:devices)
			if(dev)dev->detach_loop();
	}catch(std::exception&exc){
		log_exception(exc,"detach loop error");
	}
	if(se)fuse_session_exit(se);
	if(thread.joinable()){
		if(thread_id>0)kill(thread_id,SIGUSR1);
		thread.join();
	}
}

void netblk_implement_fuse3::init(){
	if(thread.joinable())return;
	thread=std::thread(&netblk_implement_fuse3::thread_main,this);
	sem_ready.acquire();
	running=true;
}

void netblk_implement_fuse3::run(){
	if(!thread.joinable())running=false;
	sleep(1);
}

void netblk_device_fuse3::destroy(){
	try{
		detach_loop();
	}catch(std::exception&exc){
		log_exception(exc,"detach loop failed");
	}
	try{
		if(impl.expired())return;
		auto impl=get_impl();
		std::lock_guard<std::mutex>lock(impl->mutex);
		for(auto it=impl->devices.begin();it!=impl->devices.end();it++){
			if(it->get()!=this)continue;
			impl->devices.erase(it);
			break;
		}
	}catch(std::exception&exc){
		log_exception(exc,"destroy failed");
	}
}

void netblk_device_fuse3::init(){
	{
		auto impl=get_impl();
		std::lock_guard<std::mutex>lock(impl->mutex);
		if(fuse3_filename.empty())
			fuse3_filename=std::format("image.{}",impl->devid++);
		impl->devices.push_back(shared_from_this());
		backend->prefetch(0,0x100000);
		log_info("{} initialized",fuse3_filename);
	}
	try{
		setup_loop();
	}catch(std::exception&exc){
		log_exception(exc,"{} setup loop failed",get_fuse_path());
	}
	setup_block();
}

std::string netblk_device_fuse3::get_path()const{
	if(loop_id<0)return get_fuse_path();
	return std::format("/dev/loop{}",loop_id);
}

std::string netblk_device_fuse3::get_fuse_path()const{
	auto impl=get_impl();
	if(fuse3_filename.empty())return "";
	return path_join(impl->mountpoint,fuse3_filename);
}

void netblk_device_fuse3::setup_loop(){
	auto src=get_fuse_path();
	loop_id=loop_set_file(src);
	log_info("{} setup loop{}",get_fuse_path(),loop_id);
}

void netblk_device_fuse3::detach_loop(){
	if(loop_id<0)return;
	try{
		loop_detach(loop_id);
		log_info("detached loop{}",loop_id);
		loop_id=-1;
	}catch(std::exception&exc){
		log_exception(exc,"detach loop{} error",loop_id);
	}
}

std::shared_ptr<netblk_device>netblk_implement_fuse3::create(
	const std::shared_ptr<netblk_backend>&backend
){
	auto dev=std::make_shared<netblk_device_fuse3>();
	dev->impl=shared_from_this();
	dev->backend=backend;
	dev->init();
	return dev;
}
#endif
