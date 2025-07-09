#include<netdb.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include"url.h"
#include"error.h"
#include"std-utils.h"
#include"net-utils.h"

enum ip_ver{
	IP_V4   = 1,
	IP_V6   = 2,
	IP_DUAL = IP_V4|IP_V6,
};

static size_t init_address(socket_address&sock,const url&url,void*addr,size_t len,int af){
	sock.addr.sa_family=af;
	int port=url.get_port();
	if(port<0)port=0;
	switch(af){
		case AF_INET:{
			sock.in4.sin_port=htons(port);
			memcpy(&sock.in4.sin_addr,addr,len);
			sock.len=sizeof(sockaddr_in);
		}break;
		case AF_INET6:{
			sock.in6.sin6_port=htons(port);
			memcpy(&sock.in6.sin6_addr,addr,len);
			sock.len=sizeof(sockaddr_in6);
		}break;
		default:throw RuntimeError("unexpected protocol");
	}
	return sock.len;
}

size_t url::to_sock_addr(socket_address&addr,int prefer)const{
	memset(&addr,0,sizeof(addr));
	if(prefer==0){
		if(scheme.starts_with("netlink"))
			prefer=AF_NETLINK;
		if(scheme.starts_with("unix"))
			prefer=AF_UNIX;
		if(scheme.ends_with("tcp6")||scheme.ends_with("udp6"))
			prefer=AF_INET6;
		if(scheme.ends_with("tcp4")||scheme.ends_with("udp4"))
			prefer=AF_INET;
	}
	if(prefer==AF_UNIX){
		bool abs;
		std::string name;
		auto pe=path.empty()||path=="/";
		auto he=host.empty();
		if(!he&&pe)abs=true,name=host;
		else if(he&&!pe)abs=false,name=path;
		else throw RuntimeError("bad host or path setting for unix socket");
		if(name.empty())throw RuntimeError("no unix socket address set");
		auto len=name.length()+(abs?1:0);
		if(len>=sizeof(addr.un.sun_path))
			throw RuntimeError("unix socket address too long: {}",name);
		addr.len=std::min(sizeof(sockaddr_un),sizeof(addr.un.sun_family)+len);
		addr.un.sun_family=prefer;
		strncpy(
			&addr.un.sun_path[abs?1:0],
			name.c_str(),
			sizeof(addr.un.sun_path)
		);
		return addr.len;
	}else if(prefer==AF_NETLINK){
		addr.len=sizeof(sockaddr_nl);
		addr.nl.nl_family=prefer;
		addr.nl.nl_groups=std::max(0,port);
		if(!username.empty())
			addr.nl.nl_pid=std::stoi(username);
		return addr.len;
	}
	ip_ver ver;
	char buff[64];
	switch(prefer){
		case 0:ver=IP_DUAL;break;
		case AF_INET:ver=IP_V4;break;
		case AF_INET6:ver=IP_V6;break;
		default:throw RuntimeError("unsupported prefer family");
	}
	if(host.empty())throw RuntimeError("host not set");
	hostent*he;
	if(have_bit(ver,IP_V6)&&inet_pton(AF_INET6,host.c_str(),buff)>0)
		return init_address(addr,*this,buff,sizeof(in6_addr),AF_INET6);
	if(have_bit(ver,IP_V4)&&inet_pton(AF_INET,host.c_str(),buff)>0)
		return init_address(addr,*this,buff,sizeof(in_addr),AF_INET);
	if(have_bit(ver,IP_V6)&&(he=gethostbyname2(host.c_str(),AF_INET6)))
		return init_address(addr,*this,he->h_addr,he->h_length,he->h_addrtype);
	if(have_bit(ver,IP_V4)&&(he=gethostbyname2(host.c_str(),AF_INET)))
		return init_address(addr,*this,he->h_addr,he->h_length,he->h_addrtype);
	throw RuntimeError("parse host address failed");
}
