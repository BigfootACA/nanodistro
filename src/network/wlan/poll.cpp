#include"internal.h"
#include"error.h"
#include"worker.h"
#include<cstring>
#include<sys/eventfd.h>

bool wlan_client_impl::read_event(){
	char buff[0x8000]{};
	ssize_t ret=::recv(fd,buff,sizeof(buff),0);
	if(ret<0){
		if(errno==EINTR)return true;
		if(errno==EAGAIN)return false;
		throw ErrnoError("failed to recv event");
	}else if(ret==0)
		throw RuntimeError("recv event reached EOF");
	std::string data{buff,buff+ret};
	process_data(data);
	return true;
}

void wlan_client_impl::on_event(event_handler_context*ev){
	if(ev->type!=type_events)return;
	if(ev->event&EPOLLIN)try{
		while(read_event());
	}catch(std::exception&exc){
		log_exception(exc,"failed to read event");
		ev->ev->want_remove=true;
		poll_id=0;
	}
}

void wlan_client_impl::process_data(const std::string&data){
	auto pos=std::string::npos;
	if(data.starts_with('<')&&(pos=data.find('>'))!=std::string::npos){
		auto level=std::stoi(data.substr(1,pos-1));
		auto event=data.substr(pos+1);
		handle_event(level,event);
	}else handle_data(data);
}

void wlan_client_impl::handle_event(int level,const std::string&msg){
	log_info("wlan event on {}: <{}> {}",dev,level,msg);
	std::lock_guard<std::mutex>lk(event_lock);
	for(auto&[id,cb]:on_events)
		if(cb)worker_add(std::bind(cb,id,msg));
}

void wlan_client_impl::handle_data(const std::string&data){
	std::lock_guard<std::mutex>lk(data_lock);
	buffer+=data;
	sem.release();
}

std::string wlan_client_impl::recv_(){
	if(poll_id<=0)
		throw RuntimeError("wlan poll worker not started");
	sem.acquire();
	std::lock_guard<std::mutex>lk(data_lock);
	auto ret=buffer;
	buffer.clear();
	return ret;
}

uint64_t wlan_client_impl::listen_event(const std::function<void(uint64_t id,const std::string&)>&cb){
	std::lock_guard<std::mutex>lk(event_lock);
	if(!cb)throw InvalidArgument("invalid callback");
	uint64_t id=++event_id;
	on_events[id]=cb;
	return id;
}

void wlan_client_impl::unlisten_event(uint64_t id){
	std::lock_guard<std::mutex>lk(event_lock);
	if(on_events.contains(id))on_events.erase(id);
}
