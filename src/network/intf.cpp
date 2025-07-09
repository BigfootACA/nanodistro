#include"netif.h"
#include"error.h"
#include"fs-utils.h"
#include<format>
#include<cstring>
#include<sys/ioctl.h>
#include<net/if.h>
#include<net/if_arp.h>

struct ifreq_ext{
	ifreq d{};
	ifreq_ext(
		intf_helper*intf,
		const std::string&name,
		unsigned long request,
		const ifreq&o
	);
};

ifreq_ext::ifreq_ext(
	intf_helper*intf,
	const std::string&name,
	unsigned long request,
	const ifreq&o
):d(o){
	auto ifn=intf->get_ifname();
	strncpy(d.ifr_name,ifn.c_str(),sizeof(d.ifr_name)-1);
	xioctl_(intf->get_fd(),name,request,(unsigned long)&d);
}
#define ioctl_ifreq(_var,_req,...) ifreq_ext _var(this,#_req,_req,{__VA_ARGS__})

mac intf_helper::get_hwaddr(){
	ioctl_ifreq(ifr,SIOCGIFHWADDR);
	if(ifr.d.ifr_hwaddr.sa_family!=ARPHRD_ETHER)
		throw RuntimeError("{} is not a ethernet interface",ifn);
	return mac(ifr.d.ifr_hwaddr);
}

int intf_helper::get_flags(){
	ioctl_ifreq(ifr,SIOCGIFFLAGS);
	return ifr.d.ifr_flags;
}

int intf_helper::get_mtu(){
	ioctl_ifreq(ifr,SIOCGIFMTU);
	return ifr.d.ifr_mtu;
}

int intf_helper::get_index(){
	ioctl_ifreq(ifr,SIOCGIFINDEX);
	return ifr.d.ifr_ifindex;
}

void intf_helper::set_flags(int flags){
	ifreq ifn{};
	ifn.ifr_flags=flags;
	ioctl_ifreq(ifr,SIOCSIFFLAGS,ifn);
}

bool intf_helper::is_up(){
	return is_flags(IFF_UP);
}

void intf_helper::set_up(bool up){
	int fl=IFF_UP|IFF_RUNNING;
	if(up)add_flags(fl);
	else clear_flags(fl);
}
