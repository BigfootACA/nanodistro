#include<string>
#include<cerrno>
#include<sys/socket.h>
#include<linux/netlink.h>
#include"log.h"
#include"str-utils.h"
#include"net-utils.h"
#include"event-loop.h"
#include"uevent.h"
#include"error.h"

class uevent_listener_impl:public uevent_listener{
	public:
		void start();
		bool read_uevent();
		void process_uevent(std::map<std::string,std::string>&uevent);
		void event_handler(event_handler_context*ev);
		uint64_t listen(const handler&h)override;
		void unlisten(uint64_t id)override;
		std::map<uint64_t,std::function<bool(uint64_t id,std::map<std::string,std::string>&)>>handlers{};
		uint64_t event_id=0;
		int uevent_fd=-1;
};

static uevent_listener_impl*def=nullptr;

void uevent_listener_impl::process_uevent(std::map<std::string,std::string>&uevent){
	for(auto&[id,handler]:handlers)if(handler)try{
		if(!handler(id,uevent))break;
	}catch(std::exception&exc){
		log_exception(exc,"failed to process uevent handler {}",id);
	}
}

bool uevent_listener_impl::read_uevent(){
	char buf[8192];
	errno=0;
	ssize_t len=recv(uevent_fd,buf,sizeof(buf),MSG_DONTWAIT);
	if(len<=0){
		if(len<0&&errno==EAGAIN)return false;
		if(len<0&&errno==EINTR)return true;
		throw ErrnoError("uevent recv failed");
	}
	std::string str(buf,buf+len);
	if(str.empty())return true;
	auto hidx=str.find('\0');
	if(hidx==std::string::npos)return true;
	auto uevent=parse_environ(str.substr(hidx+1),0,'=',false);
	process_uevent(uevent);
	return true;
}

void uevent_listener_impl::event_handler(event_handler_context*ev){
	if(ev->type!=type_events)return;
	if(ev->event&EPOLLIN)try{
		while(read_uevent());
	}catch(std::exception&exc){
		log_exception(exc,"failed to read uevent");
		close(uevent_fd);
		uevent_fd=-1;
		ev->ev->want_remove=true;
	}
}

void uevent_listener_impl::start(){
	uevent_fd=socket(AF_NETLINK,SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK,NETLINK_KOBJECT_UEVENT);
	if(uevent_fd<0)throw ErrnoError("failed to create uevent socket");
	socket_address sa{};
	sa.len=sizeof(sa.nl);
	sa.nl.nl_family=AF_NETLINK;
	sa.nl.nl_groups=1;
	if(bind(uevent_fd,&sa.addr,sa.len)<0)
		throw ErrnoError("failed to bind uevent socket");
	auto f=std::bind(&uevent_listener_impl::event_handler,this,std::placeholders::_1);
	event_id=event_loop::push_handler(uevent_fd,EPOLLIN,f);
}

uint64_t uevent_listener_impl::listen(const handler&h){
	if(!uevent_fd)start();
	auto id=++event_id;
	handlers[id]=h;
	return id;
}

void uevent_listener_impl::unlisten(uint64_t id){
	auto idx=handlers.find(id);
	if(idx==handlers.end())return;
	handlers.erase(idx);
}

uevent_listener*uevent_listener::get(){
	if(!def)def=new uevent_listener_impl;
	if(!def)throw RuntimeError("invalid instance");
	if(def->uevent_fd<0)def->start();
	return def;
}

uevent_listener*uevent_listener::create(){
	return new uevent_listener_impl;
}

uint64_t uevent_listener::add(const handler&h){
	return uevent_listener::get()->listen(h);
}

void uevent_listener::remove(uint64_t id){
	uevent_listener::get()->unlisten(id);
}
