#include"netif.h"
#include"error.h"
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>

void network_helper::init(){
	autoclose=true;
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0)throw ErrnoError("failed to create local socket");
}

network_helper::~network_helper(){
	if(!autoclose)return;
	close(fd);
	fd=-1;
}

intf_helper network_helper::get_intf(const std::string&ifn){
	return intf_helper(fd,ifn);
}

void netlink_helper::init(){
	autoclose=true;
	fd=socket(AF_NETLINK,SOCK_DGRAM,NETLINK_ROUTE);
	if(fd<0)throw ErrnoError("failed to create netlink socket");
}

netlink_helper::~netlink_helper(){
	if(!autoclose)return;
	close(fd);
	fd=-1;
}

size_t netlink_helper::xsend(const void*buf,size_t len){
	if(!buf||len==0)return 0;
	while(true){
		auto ret=::send(fd,buf,len,0);
		if(ret<0){
			if(errno==EINTR)continue;
			throw ErrnoError("failed to send netlink message");
		}
		if(ret==0)throw RuntimeError("netlink send reached EOF");
		return (size_t)ret;
	}
}

size_t netlink_helper::xrecv(void*buf,size_t len){
	if(!buf||len==0)return 0;
	while(true){
		auto ret=::recv(fd,buf,len,0);
		if(ret<0){
			if(errno==EINTR)continue;
			throw ErrnoError("failed to recv netlink message");
		}
		if(ret==0)throw RuntimeError("netlink recv reached EOF");
		return (size_t)ret;
	}
}
