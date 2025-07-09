#ifndef INTF_H
#define INTF_H
#include<string>
#include<vector>
#include<functional>
#include<linux/netlink.h>
#include"net-utils.h"

class intf_helper{
	public:
		inline intf_helper(){}
		inline intf_helper(int fd,const std::string&ifn):fd(fd),ifn(ifn){}
		inline std::string get_ifname()const{return ifn;}
		mac get_hwaddr();
		int get_flags();
		int get_mtu();
		int get_index();
		void set_flags(int flags);
		void add_flags(int flags){set_flags(get_flags()|flags);}
		void clear_flags(int flags){set_flags(get_flags()&~flags);}
		bool is_flags(int flags){return (get_flags()&flags)!=0;}
		bool is_up();
		void set_up(bool up);
		inline int get_fd(){return fd;}
	private:
		int fd=-1;
		std::string ifn{};
};

class network_helper{
	public:
		inline network_helper(bool init=true){if(init)this->init();}
		inline network_helper(int fd,bool autoclose=false):fd(fd),autoclose(autoclose){};
		~network_helper();
		void init();
		intf_helper get_intf(const std::string&ifn);
		inline int get_intf_flags(const std::string&ifn){return get_intf(ifn).get_flags();}
		inline int get_intf_mtu(const std::string&ifn){return get_intf(ifn).get_mtu();}
		inline int get_intf_index(const std::string&ifn){return get_intf(ifn).get_index();}
		inline mac get_intf_hwaddr(const std::string&ifn){return get_intf(ifn).get_hwaddr();}
		inline void set_intf_flags(const std::string&ifn,int flags){get_intf(ifn).set_flags(flags);}
		inline void add_intf_flags(const std::string&ifn,int flags){get_intf(ifn).add_flags(flags);}
		inline void clear_intf_flags(const std::string&ifn,int flags){get_intf(ifn).clear_flags(flags);}
		inline bool is_intf_flags(const std::string&ifn,int flags){return get_intf(ifn).is_flags(flags);}
		inline bool is_intf_up(const std::string&ifn){return get_intf(ifn).is_up();}
		inline void set_intf_up(const std::string&ifn,bool up){get_intf(ifn).set_up(up);}
	private:
		int fd=-1;
		bool autoclose=false;
};

class netlink_helper{
	public:
		inline netlink_helper(bool init=true){if(init)this->init();}
		inline netlink_helper(int fd,bool autoclose=false):fd(fd),autoclose(autoclose){};
		~netlink_helper();
		void init();
		std::vector<ipv4_cidr>get_address_v4(int ifi);
		std::vector<ipv4_cidr>get_address_v4(const std::string&ifn);
		std::vector<ipv6_cidr>get_address_v6(int ifi);
		std::vector<ipv6_cidr>get_address_v6(const std::string&ifn);
		ipv4_route_table get_routes_v4();
		ipv6_route_table get_routes_v6();
		size_t xsend(const void*buf,size_t len);
		size_t xrecv(void*buf,size_t len);
		void process_netlink(const std::function<bool(nlmsghdr*)>&cb);
	private:
		int fd=-1;
		bool autoclose=false;
};

struct intf_address_info{
	std::vector<ipv4_cidr>addr_v4{};
	std::vector<ipv6_cidr>addr_v6{};
	ipv4 gw_v4{};
	ipv6 gw_v6{};
};
extern intf_address_info get_intf_address(const std::string&ifn);
#endif
