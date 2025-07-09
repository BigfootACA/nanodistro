#include<set>
#include"netif.h"
#include"error.h"
#include<net/if.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<linux/netlink.h>
#include<linux/rtnetlink.h>

std::vector<ipv4_cidr>netlink_helper::get_address_v4(int ifi){
	std::set<ipv4_cidr>addr_set;
	struct{
		nlmsghdr nlh;
		ifaddrmsg ifa;
	}req{};
	req.nlh.nlmsg_len=NLMSG_LENGTH(sizeof(ifaddrmsg));
	req.nlh.nlmsg_type=RTM_GETADDR;
	req.nlh.nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP;
	req.ifa.ifa_family=AF_INET;
	xsend(&req,req.nlh.nlmsg_len);
	process_netlink([&](nlmsghdr*nlh){
		if(nlh->nlmsg_type!=RTM_NEWADDR)return true;
		auto ifa=(ifaddrmsg*)NLMSG_DATA(nlh);
		if(ifa->ifa_index!=ifi)return true;
		auto rta=IFA_RTA(ifa);
		int rta_len=IFA_PAYLOAD(nlh);
		for(;RTA_OK(rta,rta_len);rta=RTA_NEXT(rta,rta_len)){
			if(rta->rta_type==IFA_LOCAL){
				ipv4 addr((in_addr*)RTA_DATA(rta));
				ipv4_mask mask(ifa->ifa_prefixlen);
				addr_set.insert(ipv4_cidr(addr,mask));
			}
		}
		return true;
	});
	return std::vector<ipv4_cidr>(addr_set.begin(),addr_set.end());
}

std::vector<ipv6_cidr>netlink_helper::get_address_v6(int ifi){
	std::set<ipv6_cidr>addr_set;
	struct{
		nlmsghdr nlh;
		ifaddrmsg ifa;
	}req{};
	req.nlh.nlmsg_len=NLMSG_LENGTH(sizeof(ifaddrmsg));
	req.nlh.nlmsg_type=RTM_GETADDR;
	req.nlh.nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP;
	req.ifa.ifa_family=AF_INET6;
	xsend(&req,req.nlh.nlmsg_len);
	process_netlink([&](nlmsghdr*nlh){
		if(nlh->nlmsg_type!=RTM_NEWADDR)return true;
		auto ifa=(ifaddrmsg*)NLMSG_DATA(nlh);
		if(ifa->ifa_index!=ifi)return true;
		auto rta=IFA_RTA(ifa);
		int rta_len=IFA_PAYLOAD(nlh);
		for(;RTA_OK(rta,rta_len);rta=RTA_NEXT(rta,rta_len)){
			if(rta->rta_type==IFA_LOCAL||rta->rta_type==IFA_ADDRESS){
				ipv6 addr((in6_addr*)RTA_DATA(rta));
				int pfx=ifa->ifa_prefixlen;
				addr_set.insert(ipv6_cidr(addr,pfx));
			}
		}
		return true;
	});
	return std::vector<ipv6_cidr>(addr_set.begin(),addr_set.end());
}

std::vector<ipv4_cidr>netlink_helper::get_address_v4(const std::string&ifn){
	return get_address_v4(network_helper().get_intf_index(ifn));
}

std::vector<ipv6_cidr>netlink_helper::get_address_v6(const std::string&ifn){
	return get_address_v6(network_helper().get_intf_index(ifn));
}

static std::string ifindex_to_name(int ifi){
	char buff[64]{};
	if(ifi<0)return {};
	auto ifn=if_indextoname(ifi,buff);
	if(!ifn)throw RuntimeError("failed to get interface name for index {}",ifi);
	if(ifn[0])return ifn;
	return {};
}

ipv4_route_table netlink_helper::get_routes_v4(){
	ipv4_route_table table{};
	struct{
		nlmsghdr nlh;
		rtmsg rtm;
	}req{};
	req.nlh.nlmsg_len=NLMSG_LENGTH(sizeof(rtmsg));
	req.nlh.nlmsg_type=RTM_GETROUTE;
	req.nlh.nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP;
	req.rtm.rtm_family=AF_INET;
	xsend(&req,req.nlh.nlmsg_len);
	process_netlink([&](nlmsghdr*nlh){
		if(nlh->nlmsg_type!=RTM_NEWROUTE)return true;
		auto rtm=(rtmsg*)NLMSG_DATA(nlh);
		auto rta=RTM_RTA(rtm);
		int rtl=RTM_PAYLOAD(nlh);
		ipv4_route route{};
		bool is_mp=false;
		int oif=-1;
		route.dest.mask.prefix=rtm->rtm_dst_len;
		for(;RTA_OK(rta,rtl);rta=RTA_NEXT(rta,rtl))switch(rta->rta_type){
			case RTA_DST:route.dest.addr.set((in_addr*)RTA_DATA(rta));break;
			case RTA_GATEWAY:route.next.set((in_addr*)RTA_DATA(rta));break;
			case RTA_PRIORITY:route.metric=*(int*)RTA_DATA(rta);break;
			case RTA_OIF:oif=*(int*)RTA_DATA(rta);break;
			case RTA_MULTIPATH:{
				is_mp=true;
				auto nh=(rtnexthop*)RTA_DATA(rta);
				int len=RTA_PAYLOAD(rta);
				while(len>=(int)sizeof(*nh)&&nh->rtnh_len>=sizeof(*nh)){
					ipv4_route nr=route;
					nr.ifname=ifindex_to_name(nh->rtnh_ifindex);
					int al=nh->rtnh_len-sizeof(*nh);
					char*ap=(char*)nh+sizeof(*nh);
					for(auto a=(rtattr*)ap;RTA_OK(a,al);a=RTA_NEXT(a,al))
						if(a->rta_type==RTA_GATEWAY)
							nr.next.set((in_addr*)RTA_DATA(a));
					table.routes.push_back(nr);
					len-=NLMSG_ALIGN(nh->rtnh_len);
					nh=(struct rtnexthop*)((char*)nh + NLMSG_ALIGN(nh->rtnh_len));
				}
				break;
			}
		}
		if(is_mp&&route.next.empty())return true;
		if(oif>=0)route.ifname=ifindex_to_name(oif);
		table.routes.push_back(route);
		return true;
	});
	return table;
}

ipv6_route_table netlink_helper::get_routes_v6(){
	ipv6_route_table table{};
	struct{
		nlmsghdr nlh;
		rtmsg rtm;
	}req{};
	req.nlh.nlmsg_len=NLMSG_LENGTH(sizeof(rtmsg));
	req.nlh.nlmsg_type=RTM_GETROUTE;
	req.nlh.nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP;
	req.rtm.rtm_family=AF_INET6;
	xsend(&req,req.nlh.nlmsg_len);
	process_netlink([&](nlmsghdr*nlh){
		if(nlh->nlmsg_type!=RTM_NEWROUTE)return true;
		auto rtm=(rtmsg*)NLMSG_DATA(nlh);
		auto rta=RTM_RTA(rtm);
		int rtl=RTM_PAYLOAD(nlh);
		ipv6_route route{};
		bool is_mp=false;
		int oif=-1;
		route.dest.prefix=rtm->rtm_dst_len;
		for(;RTA_OK(rta,rtl);rta=RTA_NEXT(rta,rtl))switch(rta->rta_type){
			case RTA_DST:route.dest.addr.set((in6_addr*)RTA_DATA(rta));break;
			case RTA_GATEWAY:route.next.set((in6_addr*)RTA_DATA(rta));break;
			case RTA_PRIORITY:route.metric=*(int*)RTA_DATA(rta);break;
			case RTA_OIF:oif=*(int*)RTA_DATA(rta);break;
			case RTA_MULTIPATH:{
				is_mp=true;
				auto nh=(rtnexthop*)RTA_DATA(rta);
				int len=RTA_PAYLOAD(rta);
				while(len>=(int)sizeof(*nh)&&nh->rtnh_len>=sizeof(*nh)){
					ipv6_route nr=route;
					nr.ifname=ifindex_to_name(nh->rtnh_ifindex);
					int al=nh->rtnh_len-sizeof(*nh);
					char*ap=(char*)nh+sizeof(*nh);
					for(auto a=(rtattr*)ap;RTA_OK(a,al);a=RTA_NEXT(a,al))
						if(a->rta_type==RTA_GATEWAY)
							nr.next.set((in6_addr*)RTA_DATA(a));
					table.routes.push_back(nr);
					len-=NLMSG_ALIGN(nh->rtnh_len);
					nh=(struct rtnexthop*)((char*)nh + NLMSG_ALIGN(nh->rtnh_len));
				}
				break;
			}
		}
		if(is_mp&&route.next.empty())return true;
		if(oif>=0)route.ifname=ifindex_to_name(oif);
		table.routes.push_back(route);
		return true;
	});
	return table;
}

void netlink_helper::process_netlink(const std::function<bool(nlmsghdr*)>&cb){
	char buf[8192];
	while(true){
		auto ret=xrecv(buf,sizeof(buf));
		for(auto nlh=(nlmsghdr*)buf;NLMSG_OK(nlh,ret);nlh=NLMSG_NEXT(nlh,ret)){
			if(nlh->nlmsg_type==NLMSG_DONE)return;
			if(!cb(nlh))return;
			if(nlh->nlmsg_type==NLMSG_ERROR){
				auto err=(nlmsgerr*)NLMSG_DATA(nlh);
				if(err->error<0)throw RuntimeError("netlink error {}",-err->error);
			}
		}
	}
}

intf_address_info get_intf_address(const std::string&ifn){
	intf_address_info addr{};
	netlink_helper netlink{};
	if(ifn.empty())return {};
	try{
		addr.addr_v4=netlink.get_address_v4(ifn);
	}catch(std::exception&exc){
		addr.addr_v4.clear();
		log_exception(exc,"failed to get ipv4 address for {}",ifn);
	}
	try{
		addr.addr_v6=netlink.get_address_v6(ifn);
	}catch(std::exception&exc){
		addr.addr_v6.clear();
		log_exception(exc,"failed to get ipv6 address for {}",ifn);
	}
	try{
		addr.gw_v4=netlink.get_routes_v4().by_ifname(ifn).find(ipv4{}).next;
	}catch(ErrnoErrorImpl&exc){
		if(exc.err!=ENETUNREACH)
			log_exception(exc,"failed to get ipv4 gateway for {}",ifn);
	}catch(std::exception&exc){
		addr.gw_v4.clear();
		log_exception(exc,"failed to get ipv4 gateway for {}",ifn);
	}
	try{
		addr.gw_v6=netlink.get_routes_v6().by_ifname(ifn).find(ipv6{}).next;
	}catch(ErrnoErrorImpl&exc){
		if(exc.err!=ENETUNREACH)
			log_exception(exc,"failed to get ipv4 gateway for {}",ifn);
	}catch(std::exception&exc){
		addr.gw_v6.clear();
		log_exception(exc,"failed to get ipv6 gateway for {}",ifn);
	}
	return addr;	
}
