#include<map>
#include<list>
#include<atomic>
#include<memory>
#include<thread>
#include<sys/prctl.h>
#include<sys/eventfd.h>
#include"event-loop.h"
#include"std-utils.h"
#include"worker.h"
#include"error.h"

struct event_fd_context{
	int fd;
	epoll_event epoll;
	std::list<std::shared_ptr<event_handler_data>>handlers;
};

class event_loop_impl:public event_loop{
	friend class event_loop;
	public:
		explicit event_loop_impl(int fd);
		inline event_loop_impl():event_loop_impl(create_epoll()){}
		~event_loop_impl();
		uint64_t add_handler(int fd,uint32_t events,const event_handler&handler)override;
		uint64_t add_loop_job(const job_handler&handler)override;
		void remove_loop_job(const uint64_t&id)override;
		void remove_handler(const uint64_t&id)override;
		void close_fd(int fd)override;
		void process_one_event(epoll_event*ev,const std::shared_ptr<event_handler_data>&h);
		void process_event(epoll_event*ev)override;
		void process_events(epoll_event*evs,int count)override;
		void became_default(bool force=false)override;
		inline void interrupt()override{eventfd_write(event_fd,1);}
		void run_once_loop(int timeout=-1)override;
		int run()override;
		void thread_main();
		void stop(int r=0)override;
		void start()override;
	private:
		void poll_add_fd(const std::shared_ptr<event_handler_data>&d);
		void poll_remove_fd(const std::shared_ptr<event_handler_data>&d);
		void call_handler(event_handler_data*data,event_callback_type type,epoll_event*ev=nullptr);
		int ret=0;
		int epoll_fd=-1;
		int event_fd=-1;
		std::thread thread{};
		std::atomic<bool>running=false;
		bool thread_running=false;
		std::map<uint64_t,std::shared_ptr<event_handler_data>>handlers;
		std::map<int,std::shared_ptr<event_fd_context>>fd_handlers;
		std::map<uint64_t,job_handler>jobs{};
		std::atomic<uint64_t>autoid{0};
};

static event_loop_impl*def=nullptr;

void event_loop_impl::call_handler(event_handler_data*data,event_callback_type type,epoll_event*ev){
	event_handler_context ctx{};
	ctx.loop=this;
	ctx.ev=data;
	ctx.type=type;
	ctx.epoll=ev;
	ctx.event=ev?ev->events:0;
	data->handler(&ctx);
}

void event_loop_impl::run_once_loop(int timeout){
	int ret;
	epoll_event events[8];
	if((ret=epoll_wait(epoll_fd,events,ARRAY_SIZE(events),timeout))<0){
		if(errno==EINTR||errno==EAGAIN)return;
		throw ErrnoError("epoll failed");
	}
	process_events(events,ret);
}

int event_loop_impl::run(){
	running=true,ret=0;
	while(running){
		run_once_loop(1000);
		for(const auto&[id,func]:jobs)
			worker_add(func);
	}
	auto r=ret;
	running=true,ret=0;
	return r;
}

void event_loop_impl::thread_main(){
	if(!thread_running)return;
	prctl(PR_SET_NAME,"event-loop");
	try{
		run();
	}catch(std::exception&exc){
		log_exception(exc,"event loop thread failed");
	}
	thread_running=false;
}

void event_loop_impl::stop(int r){
	ret=r;
	running=false;
	interrupt();
	if(thread_running&&thread.joinable())
		thread.join();
}

void event_loop_impl::start(){
	if(thread_running)return;
	thread_running=true;
	auto f=std::bind(&event_loop_impl::thread_main,this);
	thread=std::thread(f);
}

void event_loop_impl::process_one_event(epoll_event*ev,const std::shared_ptr<event_handler_data>&h){
	try{
		h->event_total++;
		call_handler(h.get(),type_events,ev);
		h->error/=2;
	}catch(std::exception&exc){
		if(auto e=dynamic_cast<ErrnoErrorImpl*>(&exc);e&&e->err==EAGAIN)return;
		log_exception(exc,"error while process fd {}",h->fd);
		auto type=type_error;
		h->error++;
		if(h->error>=h->max_error){
			log_info("too many exception in fd {}, auto remove it",h->fd);;
			type=type_fatal;
		}
		try{
			call_handler(h.get(),type);
		}catch(...){}
		if(type==type_fatal)
			h->want_remove=true;
	}
	if(h->want_remove)try{
		remove_handler(h->id);
	}catch(...){}
}

void event_loop_impl::process_event(epoll_event*ev){
	auto fdc=(event_fd_context*)ev->data.ptr;
	for(auto it=fdc->handlers.begin();it!=fdc->handlers.end();){
		auto h=*it++;
		if((h->events&ev->events)!=0)
			process_one_event(ev,h);
	}
}

void event_loop_impl::process_events(epoll_event*evs,int count){
	for(int event=0;event<count;event++)
		process_event(&evs[event]);
}

event_loop*event_loop::create(){
	return new event_loop_impl;
}

event_loop*event_loop::create(int fd){
	return new event_loop_impl(fd);
}

int event_loop::create_epoll(){
	int r=epoll_create1(0);
	if(r<0)throw ErrnoError("epoll create failed");
	return r;
}

void event_loop_impl::close_fd(int fd){
	auto item=fd_handlers.find(fd);
	if(item==fd_handlers.end())return;
	for(const auto&hand:fd_handlers[fd]->handlers)
		call_handler(hand.get(),type_remove);
	int ret=epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,&item->second->epoll);
	if(ret<0&&errno!=EBADF)throw ErrnoError("epoll_ctl del failed");
	fd_handlers.erase(item);
	close(fd);
}

void event_loop_impl::poll_add_fd(const std::shared_ptr<event_handler_data>&d){
	std::shared_ptr<event_fd_context>fdc;
	auto item=fd_handlers.find(d->fd);
	if(item==fd_handlers.end()){
		fdc=std::make_shared<event_fd_context>();
		fdc->fd=d->fd;
		fdc->epoll.events=d->events;
		fdc->epoll.data.ptr=fdc.get();
		int ret=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,d->fd,&fdc->epoll);
		if(ret<0)throw ErrnoError("epoll_ctl add failed");
		fd_handlers[d->fd]=fdc;
	}else{
		fdc=item->second;
		auto new_evs=fdc->epoll.events|d->events;
		if(new_evs!=fdc->epoll.events){
			fdc->epoll.events=new_evs;
			int ret=epoll_ctl(epoll_fd,EPOLL_CTL_MOD,d->fd,&fdc->epoll);
			if(ret<0)throw ErrnoError("epoll_ctl mod failed");
		}
	}
	fdc->handlers.push_back(d);
}

void event_loop_impl::poll_remove_fd(const std::shared_ptr<event_handler_data>&d){
	std::shared_ptr<event_fd_context>fdc;
	auto item=fd_handlers.find(d->fd);
	if(item==fd_handlers.end())
		throw RuntimeError("fd not found in fd handlers");
	fdc=item->second;
	auto ev=std::find(fdc->handlers.begin(),fdc->handlers.end(),d);
	if(ev==fdc->handlers.end())
		throw RuntimeError("event not found in handlers");
	fdc->handlers.erase(ev);
	if(fdc->handlers.empty()){
		int ret=epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fdc->fd,&fdc->epoll);
		if(ret<0&&errno!=EBADF)throw ErrnoError("epoll_ctl del failed");
		fd_handlers.erase(item);
	}else{
		uint32_t new_evs=0;
		for(const auto&i:fdc->handlers)
			new_evs|=i->events;
		if(new_evs!=fdc->epoll.events){
			fdc->epoll.events=new_evs;
			int ret=epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fdc->fd,&fdc->epoll);
			if(ret<0)throw ErrnoError("epoll_ctl mod failed");
		}
	}
}

uint64_t event_loop_impl::add_handler(
	int fd,uint32_t events,
	const event_handler&handler
){
	auto ev=std::make_shared<event_handler_data>();
	ev->fd=fd,ev->events=events,ev->handler=std::move(handler);
	ev->error=0,ev->max_error=10;
	ev->id=++autoid;
	if(handlers.find(ev->id)!=handlers.end())
		throw RuntimeError("handler already exists");
	handlers[ev->id]=ev;
	poll_add_fd(ev);
	call_handler(ev.get(),type_init);
	return ev->id;
}

uint64_t event_loop_impl::add_loop_job(const job_handler&handler){
	uint64_t id=0;
	do{id=++autoid;}while(std_contains_key(jobs,id));
	jobs[id]=handler;
	return id;
}

void event_loop_impl::remove_loop_job(const uint64_t&id){
	auto idx=jobs.find(id);
	if(idx==jobs.end())return;
	jobs.erase(idx);
}

void event_loop_impl::remove_handler(const uint64_t&id){
	auto idx=handlers.find(id);
	if(idx==handlers.end())return;
	auto ev=idx->second;
	handlers.erase(idx);
	call_handler(ev.get(),type_remove);
	poll_remove_fd(ev);
}

event_loop_impl::event_loop_impl(int fd):epoll_fd(fd){
	if(!def)def=this;
	if(event_fd<0){
		event_fd=eventfd(0,EFD_SEMAPHORE|EFD_NONBLOCK|EFD_CLOEXEC);
		if(event_fd<0)throw ErrnoError("eventfd create failed");
	}
	add_handler(event_fd,EPOLLIN,[efd=event_fd](auto ev){
		if(ev->type!=type_events)return;
		eventfd_t val;
		eventfd_read(efd,&val);
	});
}

event_loop_impl::~event_loop_impl(){
	if(def==this)def=nullptr;
	if(epoll_fd>=0)close(epoll_fd);
	epoll_fd=-1;
}

void event_loop_impl::became_default(bool force){
	if(def==this)return;
	if(def&&!force)throw RuntimeError("already have a default event loop");
	def=this;
}

event_loop*event_loop::get(){
	if(!def)def=(event_loop_impl*)create();
	if(!def)throw RuntimeError("invalid instance");
	return def;
}

event_loop*event_loop::get_start(){
	auto v=(event_loop_impl*)get();
	if(!v->running)v->start();
	return v;
}

uint64_t event_loop::push_handler(
	int fd,uint32_t events,
	const event_handler&handler
){
	return get_start()->add_handler(fd,events,handler);
}

uint64_t event_loop::push_loop_job(const job_handler&handler){
	return get_start()->add_loop_job(handler);
}

void event_loop::pop_handler(uint64_t id){
	get_start()->remove_handler(id);
}
