#if defined(USE_LIB_LIBURING)&&defined(HAVE_LINUX_UBLK_CMD_H)
#include<cmath>
#include<fcntl.h>
#include<cassert>
#include<cstring>
#include<liburing.h>
#include<sys/mman.h>
#include<sys/prctl.h>
#include<sys/sysinfo.h>
#include<linux/ublk_cmd.h>
#include"internal.h"
#include"fs-utils.h"
#include"std-utils.h"
#include"cleanup.h"
#include"readable.h"
#include"modules.h"
#include"error.h"

#ifndef PR_SET_VMA
#define PR_SET_VMA 0x53564d41
#endif
#ifndef PR_SET_VMA_ANON_NAME
#define PR_SET_VMA_ANON_NAME 0
#endif

#define CTRL "/dev/ublk-control"
#define BDEVNAME "ublkb"
#define CDEVNAME "ublkc"
#define BDEVPATH "/dev/" BDEVNAME
#define CDEVPATH "/dev/" CDEVNAME
#define CTRL_CMD_HAS_DATA             1
#define CTRL_CMD_HAS_BUF              2
#define UBLKSRV_NEED_FETCH_RQ         (1UL << 0)
#define UBLKSRV_NEED_COMMIT_RQ_COMP   (1UL << 1)
#define UBLKSRV_IO_FREE               (1UL << 2)
#define UBLKSRV_QUEUE_STOPPING        (1U << 0)
#define UBLKSRV_QUEUE_IDLE            (1U << 1)
#define UBLKSRV_NO_BUF                (1U << 2)
#define UBLKSRV_ZC                    (1U << 3)

static constexpr uint32_t max_io=0x100000; /* 1MiB */

struct ublk_ctrl_cmd_data{
	uint32_t cmd_op;
	uint32_t flags;
	uint64_t data[2];
	uint64_t addr;
	uint32_t len;
};

class netblk_implement_ublk:
	public netblk_implement,
	public std::enable_shared_from_this<netblk_implement_ublk>
{
	public:
		~netblk_implement_ublk();
		inline std::string get_name()const override{return "ublk";}
		void try_enable()override;
		bool is_supported()const override;
		bool is_prefer()const override;
		void init()override;
		int ctrl_cmd(const ublk_ctrl_cmd_data&data,int dev_id);
		std::shared_ptr<netblk_device>create(const std::shared_ptr<netblk_backend>&backend)override;
		io_uring ring{};
		int ctrl_fd=-1;
};

class netblk_device_ublk;
class netblk_device_ublk_queue{
	public:
		struct ublk_io{
			void*buf=nullptr;
			size_t len=0;
			uint32_t flags=0;
			uint32_t result=0;
		};
		void thread_main();
		void queue_init();
		void queue_deinit();
		void process_io();
		const ublksrv_io_desc*get_iod(int tag);
		int complete_io(uint32_t tag,int res);
		void handle_cqe(io_uring_cqe*cqe);
		int queue_io_cmd(ublk_io*io,uint8_t tag);
		std::shared_ptr<netblk_device_ublk>get_ublk()const;
		std::weak_ptr<netblk_device_ublk>ublk;
		std::thread thread{};
		std::string name{};
		std::vector<ublk_io>ios;
		size_t io_size=0;
		int queue_depth=0;
		int id=-1;
		size_t cmd_buf_size=0;
		void*cmd_buffer;
		io_uring ring{};
		int cmd_inflight=0;
		int io_inflight=0;
		int state=0;
};

class netblk_device_ublk:
	public netblk_device,
	public std::enable_shared_from_this<netblk_device_ublk>
{
	public:
		~netblk_device_ublk();
		void init();
		void start();
		void stop();
		void del();
		void add();
		std::string get_path()const override;
		void destroy()override;
		void io_handler(netblk_device_ublk_queue&io);
		int ctrl_cmd(const ublk_ctrl_cmd_data&data);
		std::shared_ptr<netblk_backend>get_backend()override{return backend;}
		std::shared_ptr<netblk_implement_ublk>get_impl()const;
		ublk_params params{};
		ublksrv_ctrl_dev_info dev_info{};
		std::shared_ptr<netblk_backend>backend=nullptr;
		std::weak_ptr<netblk_implement_ublk>impl;
		std::vector<netblk_device_ublk_queue>queues{};
		size_t sector=0;
		int blk_fd=-1;
};

netblk_implement_ublk::~netblk_implement_ublk(){
	log_info("ublk implement destroy");
	if(ctrl_fd>=0){
		close(ctrl_fd);
		ctrl_fd=-1;
	}
}

netblk_device_ublk::~netblk_device_ublk(){
	log_info("ublk device destroy");
	destroy();
}

std::shared_ptr<netblk_implement>netblk_implement_create_ublk(const Json::Value&opts){
	return std::make_shared<netblk_implement_ublk>();
}

std::shared_ptr<netblk_implement_ublk>netblk_device_ublk::get_impl()const{
	auto impl=this->impl.lock();
	if(!impl)throw RuntimeError("ublk device implement is null");
	return impl;
}

std::shared_ptr<netblk_device_ublk>netblk_device_ublk_queue::get_ublk()const{
	auto ublk=this->ublk.lock();
	if(!ublk)throw RuntimeError("ublk device is null");
	return ublk;
}

std::string netblk_device_ublk::get_path()const{
	if(dev_info.dev_id<0)return "";
	return std::format("{}{}",BDEVPATH,dev_info.dev_id);
}

int netblk_implement_ublk::ctrl_cmd(const ublk_ctrl_cmd_data&data,int dev_id){
	io_uring_sqe*sqe;
	io_uring_cqe*cqe;
	int ret=-EINVAL;
	if(!(sqe=io_uring_get_sqe(&ring)))
		throw RuntimeError("failed to get io uring sqe");
	auto cmd=(ublksrv_ctrl_cmd*)&sqe->addr3;
	sqe->fd=ctrl_fd;
	sqe->opcode=IORING_OP_URING_CMD;
	sqe->ioprio=0;
	if (data.flags&CTRL_CMD_HAS_BUF)
		cmd->addr=data.addr,cmd->len=data.len;
	if(data.flags&CTRL_CMD_HAS_DATA)
		cmd->data[0]=data.data[0];
	cmd->dev_id=dev_id;
	cmd->queue_id=-1;
	sqe->off=data.cmd_op;
	io_uring_sqe_set_data(sqe,cmd);
	if((ret=io_uring_submit(&ring))<0)
		throw ErrnoErrorWith(ret,"failed to io uring submit cmd");
	if((ret=io_uring_wait_cqe(&ring,&cqe))<0)
		throw ErrnoErrorWith(ret,"failed to io uring wait cqe");
	io_uring_cqe_seen(&ring,cqe);
	return cqe->res;
}

int netblk_device_ublk::ctrl_cmd(const ublk_ctrl_cmd_data&data){
	return get_impl()->ctrl_cmd(data,dev_info.dev_id);
}

void netblk_implement_ublk::try_enable(){
	module_load("ublk-drv");
}

bool netblk_implement_ublk::is_supported()const{
	return fs_exists(CTRL);
}

bool netblk_implement_ublk::is_prefer()const{
	struct sysinfo info{};
	if(sysinfo(&info)<0)return false;
	return info.totalram>=0x40000000; /* 1GiB */
}

void netblk_implement_ublk::init(){
	int ret;
	if(ctrl_fd>=0)return;
	int fd=open(CTRL,O_RDWR);
	if(fd<0)throw ErrnoError("failed to open {}",CTRL);
	cleanup_func closer(std::bind(&close,fd));
	io_uring_params p{};
	p.flags=IORING_SETUP_SQE128|IORING_SETUP_CQSIZE;
	p.cq_entries=32;
	if((ret=io_uring_queue_init_params(32,&ring,&p))<0)
		throw ErrnoErrorWith(ret,"failed to init ublk io uring");
	closer.kill();
	ctrl_fd=fd;
	running=true;
}

std::shared_ptr<netblk_device>netblk_implement_ublk::create(
	const std::shared_ptr<netblk_backend>&backend
){
	if(ctrl_fd<0)throw RuntimeError("ublk not initialized");
	auto dev=std::make_shared<netblk_device_ublk>();
	dev->impl=shared_from_this();
	dev->backend=backend;
	dev->init();
	return dev;
}

void netblk_device_ublk::init(){
	int ret;
	auto size=backend->get_size();
	dev_info={
		.nr_hw_queues=4,
		.queue_depth=16,
		.max_io_buf_bytes=max_io,
		.dev_id=(uint32_t)-1,
		.ublksrv_pid=getpid(),
	};
	params={
		.len=sizeof(ublk_params),
		.types=UBLK_PARAM_TYPE_BASIC,
		.basic={
			.max_sectors=max_io>>9,
			.dev_sectors=size>>9,
		},
	};
	auto page=sysconf(_SC_PAGESIZE);
	if(sector==0)sector=page;
	params.basic.logical_bs_shift=std::log2(sector);
	params.basic.physical_bs_shift=std::log2(page);
	params.basic.io_opt_shift=params.basic.logical_bs_shift;
	params.basic.io_min_shift=params.basic.logical_bs_shift;
	if(size%page!=0)
		throw RuntimeError("backend size {} not aligned to 4KiB",size);
	if(!backend->can_access(S_IROTH))
		throw RuntimeError("backend not accessible");
	if(!backend->can_access(S_IWOTH))
		params.basic.attrs|=UBLK_ATTR_READ_ONLY;
	backend->prefetch(0,max_io);
	add();
	auto path=std::format("{}{}",CDEVPATH,dev_info.dev_id);
	blk_fd=open(path.c_str(),O_RDWR|O_CLOEXEC);
	if(blk_fd<0)throw ErrnoError("failed to open {}",path);
	queues.resize(dev_info.nr_hw_queues);
	for(int i=0;i<dev_info.nr_hw_queues;i++){
		queues[i].ublk=shared_from_this();
		queues[i].id=i;
		queues[i].name=std::format("ublk-{}-{}",dev_info.dev_id,i);
		queues[i].thread=std::thread(&netblk_device_ublk_queue::thread_main,&queues[i]);
	}
	if((ret=ctrl_cmd({
		.cmd_op=UBLK_CMD_SET_PARAMS,
		.flags=CTRL_CMD_HAS_BUF,
		.addr=(uintptr_t)&params,
		.len=sizeof(params),
	}))<0)throw ErrnoErrorWith(ret,"failed to set ublk params");
	start();
	log_info("ublk device {} is ready",get_path());
	setup_block();
}

void netblk_device_ublk::start(){
	int ret;
	if((ret=ctrl_cmd({
		.cmd_op=UBLK_CMD_START_DEV,
		.flags=CTRL_CMD_HAS_DATA,
		.data={(uint64_t)getpid(),0},
	}))<0)throw ErrnoErrorWith(ret,"failed to start ublk device");
	log_info("ublk device {} started",get_path()	);
}

void netblk_device_ublk::stop(){
	int ret;
	if((ret=ctrl_cmd({
		.cmd_op=UBLK_CMD_STOP_DEV,
		.flags=CTRL_CMD_HAS_DATA,
	}))<0)throw ErrnoErrorWith(ret,"failed to stop ublk device");
	log_info("ublk device {} stopped",get_path());
}

void netblk_device_ublk::add(){
	int ret;
	if((ret=ctrl_cmd({
		.cmd_op=UBLK_CMD_ADD_DEV,
		.flags=CTRL_CMD_HAS_BUF,
		.addr=(uintptr_t)&dev_info,
		.len=sizeof(dev_info),
	}))<0)throw ErrnoErrorWith(ret,"failed to add ublk device");
	log_info("ublk device added as {}",dev_info.dev_id);
}

void netblk_device_ublk::del(){
	int ret;
	if((ret=ctrl_cmd({.cmd_op=UBLK_CMD_DEL_DEV}))<0)
		throw ErrnoErrorWith(ret,"failed to del ublk device");
	log_info("ublk device deleted {}",dev_info.dev_id);
}

void netblk_device_ublk::destroy(){
	close(blk_fd);
	try{del();}catch(...){}
	for(auto&queue:queues)
		queue.state|=UBLKSRV_QUEUE_STOPPING;
	for(auto&queue:queues)if(queue.thread.joinable())
		queue.thread.join();
}

void netblk_device_ublk_queue::queue_init(){
	int ret;
	uint64_t off;
	auto ublk=get_ublk();
	cmd_buf_size=align_up(queue_depth*sizeof(ublksrv_io_desc),0x1000);
	off=UBLKSRV_CMD_BUF_OFFSET+id*(UBLK_MAX_QUEUE_DEPTH*sizeof(ublksrv_io_desc));
	cmd_buffer=mmap(nullptr,cmd_buf_size,PROT_READ,MAP_SHARED|MAP_POPULATE,ublk->blk_fd,off);
	if(!cmd_buffer||cmd_buffer==MAP_FAILED)throw ErrnoError("failed to mmap ublk cmd buffer");
	prctl(
		PR_SET_VMA,PR_SET_VMA_ANON_NAME,
		(uintptr_t)cmd_buffer,cmd_buf_size,
		std::format("ublk-cmd-{}-{}",ublk->dev_info.dev_id,id).c_str()
	);
	log_debug(
		"allocated ublk cmd buffer {} offset 0x{:x} at 0x{:x}",
		size_string_float(cmd_buf_size),off,(uintptr_t)cmd_buffer
	);
	for(int i=0;i<queue_depth;i++){
		ublk_io io{};
		io.buf=mmap(nullptr,io_size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
		if(!io.buf||io.buf==MAP_FAILED)throw ErrnoError("failed to mmap ublk io buffer");
		prctl(
			PR_SET_VMA,PR_SET_VMA_ANON_NAME,
			(uintptr_t)io.buf,io_size,
			std::format("ublk-io-{}-{}-{}",ublk->dev_info.dev_id,id,i).c_str()
		);
		log_debug(
			"allocated ublk io buffer {} at 0x{:x}",
			size_string_float(io_size),(uintptr_t)io.buf
		);
		io.flags=UBLKSRV_NEED_FETCH_RQ|UBLKSRV_IO_FREE;
		ios.push_back(io);
	}
	io_uring_params p{};
	p.flags=IORING_SETUP_SQE128|IORING_SETUP_COOP_TASKRUN|IORING_SETUP_CQSIZE;
	p.cq_entries=queue_depth;
	ret=io_uring_queue_init_params(queue_depth,&ring,&p);
	if(ret<0)throw ErrnoErrorWith(ret,"failed to init ublk io queue uring");
	io_uring_register_ring_fd(&ring);
	ret=io_uring_register_files(&ring,&ublk->blk_fd,1);
	if(ret<0)throw ErrnoErrorWith(ret,"failed to register ublk fd");
}

void netblk_device_ublk_queue::queue_deinit(){
	io_uring_unregister_ring_fd(&ring);
	if(ring.ring_fd>0){
		io_uring_unregister_files(&ring);
		close(ring.ring_fd);
		ring.ring_fd=-1;
	}
	if(cmd_buffer&&cmd_buffer!=MAP_FAILED){
		munmap(cmd_buffer,cmd_buf_size);
		cmd_buffer=nullptr;
	}
	for(auto&io:ios){
		if(io.buf&&io.buf!=MAP_FAILED)
			munmap(io.buf,io_size);
		io.buf=nullptr;
	}
}

int netblk_device_ublk_queue::queue_io_cmd(ublk_io*io,uint8_t tag){
	ublksrv_io_cmd*cmd;
	io_uring_sqe*sqe;
	uint32_t cmd_op=0;
	uint64_t user_data;
	if(!(io->flags&UBLKSRV_IO_FREE))return 0;
	if(!(io->flags&(UBLKSRV_NEED_FETCH_RQ|UBLKSRV_NEED_COMMIT_RQ_COMP)))return 0;
	if(io->flags&UBLKSRV_NEED_COMMIT_RQ_COMP)
		cmd_op=UBLK_IO_COMMIT_AND_FETCH_REQ;
	else if(io->flags&UBLKSRV_NEED_FETCH_RQ)
		cmd_op=UBLK_IO_FETCH_REQ;
	sqe=io_uring_get_sqe(&ring);
	if(!sqe)throw RuntimeError("run out of sqe {}, tag {}",id,tag);
	cmd=(ublksrv_io_cmd*)&sqe->addr3;
	if(cmd_op==UBLK_IO_COMMIT_AND_FETCH_REQ)
		cmd->result=io->result;
	sqe->off=cmd_op;
	sqe->fd=0;
	sqe->opcode=IORING_OP_URING_CMD;
	sqe->flags=IOSQE_FIXED_FILE;
	sqe->rw_flags=0;
	cmd->tag=tag;
	cmd->addr=(uintptr_t)io->buf;
	cmd->q_id=id;
	user_data=tag|(cmd_op<<16);
	io_uring_sqe_set_data64(sqe, user_data);
	io->flags=0;
	cmd_inflight+=1;
	return 1;
}

void netblk_device_ublk_queue::thread_main(){
	prctl(PR_SET_NAME,name.c_str());
	try{
		{
			auto ublk=get_ublk();
			io_size=ublk->dev_info.max_io_buf_bytes;
			queue_depth=ublk->dev_info.queue_depth;
		}
		queue_init();
		ios.resize(queue_depth);
		for(int i=0;i<queue_depth;i++)
			queue_io_cmd(&ios[i],i);
		log_info("ublk device {} queue {} started",get_ublk()->get_path(),id);
		while(true)process_io();
	}catch(ErrnoErrorImpl&exc){
		if(exc.err==EINTR){
			log_info("ublk device queue thread {} stopping",gettid());
		}else{
			log_exception(exc,"ublk device queue {} thread error",id);
		}
	}catch(std::exception&exc){
		log_exception(exc,"ublk device queue {} thread error",id);
	}
	queue_deinit();
}

void netblk_device_ublk_queue::process_io(){
	int ret;
	io_uring_cqe*cqe1=nullptr;
	__kernel_timespec ts={.tv_sec=1,.tv_nsec=0};
	if((state&UBLKSRV_QUEUE_STOPPING)&&!io_inflight)
		throw ErrnoErrorWith(EINTR,"queue stopping");
	ret=io_uring_submit_and_wait_timeout(&ring,&cqe1,1,&ts,NULL);
	uint32_t head,count=0;
	io_uring_cqe*cqe2;
	io_uring_for_each_cqe(&ring,head,cqe2){
		handle_cqe(cqe2);
		count++;
	}
	io_uring_cq_advance(&ring, count);
	if((state&UBLKSRV_QUEUE_STOPPING))return;
	if(ret==-ETIME&&count==0&&!io_inflight){
		if((state&UBLKSRV_QUEUE_IDLE))return;
		for(int i=0;i<queue_depth;i++)
			madvise(ios[i].buf,io_size,MADV_DONTNEED);
		state|=UBLKSRV_QUEUE_IDLE;
	}else if(state&UBLKSRV_QUEUE_IDLE)
		state&=~UBLKSRV_QUEUE_IDLE;
}

int netblk_device_ublk_queue::complete_io(uint32_t tag,int res){
	ios[tag].flags|=(UBLKSRV_NEED_COMMIT_RQ_COMP|UBLKSRV_IO_FREE);
	ios[tag].result=res;
	return queue_io_cmd(&ios[tag],tag);
}

const ublksrv_io_desc*netblk_device_ublk_queue::get_iod(int tag){
	auto b=(char*)cmd_buffer;
	auto off=tag*sizeof(ublksrv_io_desc);
	return (ublksrv_io_desc*)&(b[off]);
}

void netblk_device_ublk_queue::handle_cqe(io_uring_cqe*cqe){
	uint32_t tag=cqe->user_data&0xffff;
	bool fetch=(cqe->res!=UBLK_IO_RES_ABORT)&&!(state&UBLKSRV_QUEUE_STOPPING);
	if(cqe->res<0&&cqe->res!=-ENODEV)log_error(
		"res {} userdata {:x} queue state {:x}",
		cqe->res, cqe->user_data, (int)state
	);
	cmd_inflight--;
	if(!fetch){
		state|=UBLKSRV_QUEUE_STOPPING;
		ios[tag].flags&=~UBLKSRV_NEED_FETCH_RQ;
	}
	if(cqe->res==UBLK_IO_RES_OK){
		int ret=-ENOTSUP;
		assert(tag<get_ublk()->dev_info.queue_depth);
		auto iod=get_iod(tag);
		uint32_t ublk_op=ublksrv_get_op(iod);
		auto off=iod->start_sector<<9;
		auto buff=(void*)iod->addr;
		auto size=iod->nr_sectors<<9;
		auto backend=get_ublk()->backend;
		if(ublk_op==UBLK_IO_OP_READ)
			ret=backend->do_read(off,buff,size);
		if(ublk_op==UBLK_IO_OP_WRITE)
			ret=backend->do_write(off,buff,size);
		complete_io(tag,ret);
	}else{
		ios[tag].flags=UBLKSRV_IO_FREE;
	}
}

#endif
