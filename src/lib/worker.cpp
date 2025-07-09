#include<queue>
#include<mutex>
#include<thread>
#include<memory>
#include<condition_variable>
#include<unistd.h>
#include<sys/prctl.h>
#include"worker.h"
#include"log.h"
#include"error.h"

struct worker_ctx{
	worker_ctx(uint64_t id);
	~worker_ctx();
	uint64_t id{};
	std::jthread thread{};	
	std::string name{};
	std::atomic<bool>idle=false;
	void main(std::stop_token stop);
};

struct workers_ctx{
	~workers_ctx();
	void add(const std::function<void(void)>&cb);
	std::shared_ptr<worker_ctx>create();
	void auto_create();
	void init(int workers);
	size_t max_workers=0;
	std::queue<std::function<void(void)>>queue{};
	std::mutex lock{};
	std::condition_variable cond{};
	std::vector<std::shared_ptr<worker_ctx>>workers{};
};
static workers_ctx ctx{};
static bool ctx_valid=true;

void worker_ctx::main(std::stop_token stop){
	prctl(PR_SET_NAME,name.c_str());
	log_info("worker {} started as {}",name,gettid());
	while(true){
		std::function<void(void)>cb;
		{
			std::unique_lock<std::mutex>lk(ctx.lock);
			idle=true;
			if(ctx.queue.empty()){
				if(stop.stop_requested())break;
				ctx.cond.wait(lk);
				if(ctx.queue.empty())continue;
			}
			idle=false;
			cb=ctx.queue.front();
			ctx.queue.pop();
		}
		if(cb)try{
			cb();
		}catch(std::exception&exc){
			log_exception(exc,"unexpected exception in worker");
		}
	}
	idle=false;
}

workers_ctx::~workers_ctx(){
	for(auto&worker:workers)if(worker)
		worker->thread.request_stop();
	cond.notify_all();
	for(auto&worker:workers)if(worker)
		worker->thread.join();
	workers.clear();	
	ctx_valid=false;
}

worker_ctx::~worker_ctx(){
	if(thread.joinable())
		thread.request_stop();
	ctx.cond.notify_all();
	if(thread.joinable())
		thread.join();
}

worker_ctx::worker_ctx(uint64_t id){
	this->id=id;
	this->name=std::format("worker-{}",id);
	auto f=std::bind(&worker_ctx::main,this,std::placeholders::_1);
	this->thread=std::jthread(f);
}

void workers_ctx::auto_create(){
	std::lock_guard<std::mutex>lk(lock);
	if(max_workers==0){
		auto ret=sysconf(_SC_NPROCESSORS_CONF);
		if(ret<0)throw RuntimeError("failed to get cpu count");
		max_workers=std::clamp<int>(ret,1,8);
	}
	size_t idles=0;
	for(auto&worker:workers)
		if(worker&&worker->idle)
			idles++;
	if(idles<=0&&workers.size()<max_workers)
		create();
}

void workers_ctx::add(const std::function<void(void)>&cb){
	auto_create();
	{
		std::lock_guard<std::mutex>lk(lock);
		queue.push(cb);
	}
	cond.notify_one();
}

std::shared_ptr<worker_ctx>workers_ctx::create(){
	auto id=workers.size();
	auto wctx=std::make_shared<worker_ctx>(id);
	workers.push_back(wctx);
	return wctx;
}

void workers_ctx::init(int workers){
	max_workers=workers;
}

void worker_add(const std::function<void(void)>&cb){
	if(!ctx_valid)return;
	ctx.add(cb);
}

std::shared_ptr<worker_ctx>worker_create(){
	if(!ctx_valid)return nullptr;
	return ctx.create();
}

void worker_init(int workers){
	if(!ctx_valid)return;
	ctx.init(workers);
}
