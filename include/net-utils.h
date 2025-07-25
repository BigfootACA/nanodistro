#ifndef NETWORK_H
#define NETWORK_H
#include<string>
#include<vector>
#include<cstdint>
#include<sys/un.h>
#include<netinet/in.h>
#include<linux/netlink.h>

struct socket_address{
	socklen_t len;
	uint32_t:32;
	union{
		sockaddr addr;
		sockaddr_in in4;
		sockaddr_in6 in6;
		sockaddr_un un;
		sockaddr_nl nl;
		sockaddr_storage stor;
	};
};
static_assert(sizeof(socket_address)==sizeof(socklen_t)+4+sizeof(sockaddr_storage),"socket_address size mismatch");

struct mac{
	uint8_t d[6];
	mac();
	mac(const std::string&n);
	mac(const char*n);
	mac(const sockaddr&sa);
	mac(const sockaddr*sa);
	mac(const socket_address&sa);
	mac(const socket_address*sa);
	void clear();
	void set(const mac&n);
	void set(const std::string&n);
	void set(const char*n);
	void set(const sockaddr&sa);
	void set(const sockaddr*sa);
	void set(const socket_address&sa);
	void set(const socket_address*sa);
	[[nodiscard]] int compare(const mac&other)const;
	[[nodiscard]] bool equals(const mac&other)const;
	[[nodiscard]] bool empty()const;
	[[nodiscard]] std::string to_string(const std::string&sep=":",bool upper=false)const;
	[[nodiscard]] static mac parse(const std::string&mac);
	[[nodiscard]] bool operator==(const mac&other)const;
	[[nodiscard]] bool operator!=(const mac&other)const;
	[[nodiscard]] bool operator<(const mac&other)const;
	[[nodiscard]] bool operator>(const mac&other)const;
	[[nodiscard]] bool operator<=(const mac&other)const;
	[[nodiscard]] bool operator>=(const mac&other)const;
	[[nodiscard]] bool operator==(const std::string&other)const;
	[[nodiscard]] bool operator!=(const std::string&other)const;
	[[nodiscard]] bool operator<(const std::string&other)const;
	[[nodiscard]] bool operator>(const std::string&other)const;
	[[nodiscard]] bool operator<=(const std::string&other)const;
	[[nodiscard]] bool operator>=(const std::string&other)const;
	[[nodiscard]] bool operator==(const char*other)const;
	[[nodiscard]] bool operator!=(const char*other)const;
	[[nodiscard]] bool operator<(const char*other)const;
	[[nodiscard]] bool operator>(const char*other)const;
	[[nodiscard]] bool operator<=(const char*other)const;
	[[nodiscard]] bool operator>=(const char*other)const;
	[[nodiscard]] operator std::string()const;
};
static_assert(sizeof(mac)==6,"mac address size is not 6 bytes");

struct ipv4{
	union{
		uint8_t d[4];
		uint16_t d16[2];
		uint32_t v;
	};
	ipv4();
	ipv4(const std::string&ip);
	ipv4(const char*ip);
	ipv4(uint32_t ip);
	ipv4(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4);
	ipv4(const sockaddr&sa);
	ipv4(const sockaddr*sa);
	ipv4(const sockaddr_in&sa);
	ipv4(const sockaddr_in*sa);
	ipv4(const socket_address&sa);
	ipv4(const socket_address*sa);
	ipv4(const in_addr&ia);
	ipv4(const in_addr*ia);
	ipv4 next();
	ipv4 prev();
	void clear();
	void set(const ipv4&ip);
	void set(const std::string&ip);
	void set(const char*ip);
	void set(uint32_t ip);
	void set(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4);
	void set(const sockaddr&sa);
	void set(const sockaddr*sa);
	void set(const sockaddr_in&sa);
	void set(const sockaddr_in*sa);
	void set(const socket_address&sa);
	void set(const socket_address*sa);
	void set(const in_addr&ia);
	void set(const in_addr*ia);
	[[nodiscard]] int compare(const ipv4&other)const;
	[[nodiscard]] bool equals(const ipv4&other)const;
	[[nodiscard]] std::string to_string()const;
	[[nodiscard]] in_addr to_in_addr()const;
	[[nodiscard]] sockaddr_in to_sockaddr_in()const;
	[[nodiscard]] sockaddr to_sockaddr()const;
	[[nodiscard]] socket_address to_socket_address()const;
	[[nodiscard]] bool empty()const;
	[[nodiscard]] static ipv4 parse(const std::string&ip);
	[[nodiscard]] bool operator==(const ipv4&other)const;
	[[nodiscard]] bool operator!=(const ipv4&other)const;
	[[nodiscard]] bool operator<(const ipv4&other)const;
	[[nodiscard]] bool operator>(const ipv4&other)const;
	[[nodiscard]] bool operator<=(const ipv4&other)const;
	[[nodiscard]] bool operator>=(const ipv4&other)const;
	[[nodiscard]] bool operator==(const std::string&other)const;
	[[nodiscard]] bool operator!=(const std::string&other)const;
	[[nodiscard]] bool operator<(const std::string&other)const;
	[[nodiscard]] bool operator>(const std::string&other)const;
	[[nodiscard]] bool operator<=(const std::string&other)const;
	[[nodiscard]] bool operator>=(const std::string&other)const;
	[[nodiscard]] bool operator==(const char*other)const;
	[[nodiscard]] bool operator!=(const char*other)const;
	[[nodiscard]] bool operator<(const char*other)const;
	[[nodiscard]] bool operator>(const char*other)const;
	[[nodiscard]] bool operator<=(const char*other)const;
	[[nodiscard]] bool operator>=(const char*other)const;
	[[nodiscard]] operator std::string()const;
	[[nodiscard]] operator uint32_t()const;
	[[nodiscard]] operator in_addr()const;
	[[nodiscard]] operator sockaddr_in()const;
	[[nodiscard]] operator sockaddr()const;
	[[nodiscard]] operator socket_address()const;
};
static_assert(sizeof(ipv4)==4,"ipv4 address size mismatch");

struct ipv6{
	union{
		uint8_t d8[16];
		uint16_t d16[8];
		uint32_t d32[4];
		uint64_t d64[2];
	};
	ipv6();
	ipv6(const std::string&ip);
	ipv6(const char*ip);
	ipv6(const sockaddr&sa);
	ipv6(const sockaddr*sa);
	ipv6(const sockaddr_in6&sa);
	ipv6(const sockaddr_in6*sa);
	ipv6(const socket_address&sa);
	ipv6(const socket_address*sa);
	ipv6(const in6_addr&ia);
	ipv6(const in6_addr*ia);
	ipv6 next();
	ipv6 prev();
	void clear();
	void set(const ipv6&ip);
	void set(const std::string&ip);
	void set(const char*ip);
	void set(const sockaddr&sa);
	void set(const sockaddr*sa);
	void set(const sockaddr_in6&sa);
	void set(const sockaddr_in6*sa);
	void set(const socket_address&sa);
	void set(const socket_address*sa);
	void set(const in6_addr&ia);
	void set(const in6_addr*ia);
	ipv4 to_ipv4()const;
	[[nodiscard]] int compare(const ipv6&other)const;
	[[nodiscard]] bool equals(const ipv6&other)const;
	[[nodiscard]] std::string to_string(bool upper=false)const;
	[[nodiscard]] in6_addr to_in6_addr()const;
	[[nodiscard]] sockaddr_in6 to_sockaddr_in6()const;
	[[nodiscard]] sockaddr to_sockaddr()const;
	[[nodiscard]] socket_address to_socket_address()const;
	[[nodiscard]] bool empty()const;
	[[nodiscard]] static ipv6 parse(const std::string&ip);
	[[nodiscard]] bool operator==(const ipv6&other)const;
	[[nodiscard]] bool operator!=(const ipv6&other)const;
	[[nodiscard]] bool operator<(const ipv6&other)const;
	[[nodiscard]] bool operator>(const ipv6&other)const;
	[[nodiscard]] bool operator<=(const ipv6&other)const;
	[[nodiscard]] bool operator>=(const ipv6&other)const;
	[[nodiscard]] bool operator==(const std::string&other)const;
	[[nodiscard]] bool operator!=(const std::string&other)const;
	[[nodiscard]] bool operator<(const std::string&other)const;
	[[nodiscard]] bool operator>(const std::string&other)const;
	[[nodiscard]] bool operator<=(const std::string&other)const;
	[[nodiscard]] bool operator>=(const std::string&other)const;
	[[nodiscard]] bool operator==(const char*other)const;
	[[nodiscard]] bool operator!=(const char*other)const;
	[[nodiscard]] bool operator<(const char*other)const;
	[[nodiscard]] bool operator>(const char*other)const;
	[[nodiscard]] bool operator<=(const char*other)const;
	[[nodiscard]] bool operator>=(const char*other)const;
	[[nodiscard]] operator std::string()const;
	[[nodiscard]] operator in6_addr()const;
	[[nodiscard]] operator sockaddr_in6()const;
	[[nodiscard]] operator sockaddr()const;
	[[nodiscard]] operator socket_address()const;
	[[nodiscard]] operator ipv4()const;
};
static_assert(sizeof(ipv6)==16,"ipv6 address size mismatch");

struct ipv4_mask{
	int prefix=0;
	ipv4_mask();
	ipv4_mask(const std::string&cidr);
	ipv4_mask(const char*cidr);
	ipv4_mask(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4);
	explicit ipv4_mask(int prefix);
	void clear();
	void set(const std::string&cidr);
	void set(const char*cidr);
	void set(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4);
	void set(int prefix);
	void set(const ipv4&v);
	void set(const ipv4_mask&v);
	void set_netmask(const ipv4&mask);
	void set_wildcard(const ipv4&mask);
	[[nodiscard]] int compare(const ipv4_mask&other)const;
	[[nodiscard]] bool equals(const ipv4_mask&other)const;
	[[nodiscard]] std::string to_string()const;
	[[nodiscard]] in_addr to_in_addr()const;
	[[nodiscard]] sockaddr_in to_sockaddr_in()const;
	[[nodiscard]] sockaddr to_sockaddr()const;
	[[nodiscard]] socket_address to_socket_address()const;
	[[nodiscard]] ipv4 to_netmask()const;
	[[nodiscard]] ipv4 to_wildcard()const;
	[[nodiscard]] bool empty()const;
	[[nodiscard]] static ipv4_mask parse(const std::string&ip);
	[[nodiscard]] bool operator==(const ipv4_mask&other)const;
	[[nodiscard]] bool operator!=(const ipv4_mask&other)const;
	[[nodiscard]] bool operator<(const ipv4_mask&other)const;
	[[nodiscard]] bool operator>(const ipv4_mask&other)const;
	[[nodiscard]] bool operator<=(const ipv4_mask&other)const;
	[[nodiscard]] bool operator>=(const ipv4_mask&other)const;
	[[nodiscard]] bool operator==(const std::string&other)const;
	[[nodiscard]] bool operator!=(const std::string&other)const;
	[[nodiscard]] bool operator<(const std::string&other)const;
	[[nodiscard]] bool operator>(const std::string&other)const;
	[[nodiscard]] bool operator<=(const std::string&other)const;
	[[nodiscard]] bool operator>=(const std::string&other)const;
	[[nodiscard]] bool operator==(const char*other)const;
	[[nodiscard]] bool operator!=(const char*other)const;
	[[nodiscard]] bool operator<(const char*other)const;
	[[nodiscard]] bool operator>(const char*other)const;
	[[nodiscard]] bool operator<=(const char*other)const;
	[[nodiscard]] bool operator>=(const char*other)const;
	[[nodiscard]] operator std::string()const;
	[[nodiscard]] operator uint32_t()const;
	[[nodiscard]] operator in_addr()const;
	[[nodiscard]] operator sockaddr_in()const;
	[[nodiscard]] operator sockaddr()const;
	[[nodiscard]] operator socket_address()const;
};

struct ipv4_cidr{
	ipv4 addr{};
	ipv4_mask mask{};
	ipv4_cidr();
	ipv4_cidr(const std::string&cidr);
	ipv4_cidr(const char*cidr);
	ipv4_cidr(const ipv4&addr,int prefix);
	ipv4_cidr(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4,int prefix);
	void clear();
	void set(const std::string&cidr);
	void set(const char*cidr);
	void set(const ipv4&addr,int prefix);
	void set(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4,int prefix);
	[[nodiscard]] ipv4 begin()const;
	[[nodiscard]] ipv4 end()const;
	[[nodiscard]] ipv4 net()const;
	[[nodiscard]] ipv4 brd()const;
	[[nodiscard]] bool is_addr_in(const ipv4&addr)const;
	[[nodiscard]] int compare(const ipv4_cidr&other)const;
	[[nodiscard]] bool equals(const ipv4_cidr&other)const;
	[[nodiscard]] std::string to_string()const;
	[[nodiscard]] in_addr to_in_addr()const;
	[[nodiscard]] sockaddr_in to_sockaddr_in()const;
	[[nodiscard]] sockaddr to_sockaddr()const;
	[[nodiscard]] socket_address to_socket_address()const;
	[[nodiscard]] bool empty()const;
	[[nodiscard]] static ipv4_cidr parse(const std::string&ip);
	[[nodiscard]] bool operator==(const ipv4_cidr&other)const;
	[[nodiscard]] bool operator!=(const ipv4_cidr&other)const;
	[[nodiscard]] bool operator<(const ipv4_cidr&other)const;
	[[nodiscard]] bool operator>(const ipv4_cidr&other)const;
	[[nodiscard]] bool operator<=(const ipv4_cidr&other)const;
	[[nodiscard]] bool operator>=(const ipv4_cidr&other)const;
	[[nodiscard]] bool operator==(const std::string&other)const;
	[[nodiscard]] bool operator!=(const std::string&other)const;
	[[nodiscard]] bool operator<(const std::string&other)const;
	[[nodiscard]] bool operator>(const std::string&other)const;
	[[nodiscard]] bool operator<=(const std::string&other)const;
	[[nodiscard]] bool operator>=(const std::string&other)const;
	[[nodiscard]] bool operator==(const char*other)const;
	[[nodiscard]] bool operator!=(const char*other)const;
	[[nodiscard]] bool operator<(const char*other)const;
	[[nodiscard]] bool operator>(const char*other)const;
	[[nodiscard]] bool operator<=(const char*other)const;
	[[nodiscard]] bool operator>=(const char*other)const;
	[[nodiscard]] operator std::string()const;
	[[nodiscard]] operator in_addr()const;
	[[nodiscard]] operator sockaddr_in()const;
	[[nodiscard]] operator sockaddr()const;
	[[nodiscard]] operator socket_address()const;
};

struct ipv6_cidr{
	ipv6 addr{};
	int prefix{};
	ipv6_cidr();
	ipv6_cidr(const std::string&cidr);
	ipv6_cidr(const char*cidr);
	ipv6_cidr(const ipv6&addr,int prefix);
	void clear();
	void set(const std::string&cidr);
	void set(const char*cidr);
	void set(const ipv6&addr,int prefix);
	[[nodiscard]] ipv6 begin()const;
	[[nodiscard]] ipv6 end()const;
	[[nodiscard]] ipv6 net()const;
	[[nodiscard]] ipv6 brd()const;
	[[nodiscard]] bool is_addr_in(const ipv6&addr)const;
	[[nodiscard]] int compare(const ipv6_cidr&other)const;
	[[nodiscard]] bool equals(const ipv6_cidr&other)const;
	[[nodiscard]] std::string to_string()const;
	[[nodiscard]] in6_addr to_in6_addr()const;
	[[nodiscard]] sockaddr_in6 to_sockaddr_in6()const;
	[[nodiscard]] sockaddr to_sockaddr()const;
	[[nodiscard]] socket_address to_socket_address()const;
	[[nodiscard]] bool empty()const;
	[[nodiscard]] static ipv6_cidr parse(const std::string&ip);
	[[nodiscard]] bool operator==(const ipv6_cidr&other)const;
	[[nodiscard]] bool operator!=(const ipv6_cidr&other)const;
	[[nodiscard]] bool operator<(const ipv6_cidr&other)const;
	[[nodiscard]] bool operator>(const ipv6_cidr&other)const;
	[[nodiscard]] bool operator<=(const ipv6_cidr&other)const;
	[[nodiscard]] bool operator>=(const ipv6_cidr&other)const;
	[[nodiscard]] bool operator==(const std::string&other)const;
	[[nodiscard]] bool operator!=(const std::string&other)const;
	[[nodiscard]] bool operator<(const std::string&other)const;
	[[nodiscard]] bool operator>(const std::string&other)const;
	[[nodiscard]] bool operator<=(const std::string&other)const;
	[[nodiscard]] bool operator>=(const std::string&other)const;
	[[nodiscard]] bool operator==(const char*other)const;
	[[nodiscard]] bool operator!=(const char*other)const;
	[[nodiscard]] bool operator<(const char*other)const;
	[[nodiscard]] bool operator>(const char*other)const;
	[[nodiscard]] bool operator<=(const char*other)const;
	[[nodiscard]] bool operator>=(const char*other)const;
	[[nodiscard]] operator std::string()const;
	[[nodiscard]] operator in6_addr()const;
	[[nodiscard]] operator sockaddr_in6()const;
	[[nodiscard]] operator sockaddr()const;
	[[nodiscard]] operator socket_address()const;
};

struct ipv4_route{
	ipv4_cidr dest{};
	ipv4 next{};
	std::string ifname{};
	int metric=-1;
	ipv4_route();
	void clear();
	[[nodiscard]] int compare(const ipv4_route&other)const;
	[[nodiscard]] bool equals(const ipv4_route&other)const;
	[[nodiscard]] std::string to_string()const;
	[[nodiscard]] bool empty()const;
	[[nodiscard]] bool operator==(const ipv4_route&other)const;
	[[nodiscard]] bool operator!=(const ipv4_route&other)const;
	[[nodiscard]] bool operator<(const ipv4_route&other)const;
	[[nodiscard]] bool operator>(const ipv4_route&other)const;
	[[nodiscard]] bool operator<=(const ipv4_route&other)const;
	[[nodiscard]] bool operator>=(const ipv4_route&other)const;
};

struct ipv4_route_table{
	std::vector<ipv4_route>routes{};
	const ipv4_route&find(const ipv4&dest)const;
	ipv4_route&find(const ipv4&dest);
	ipv4_route_table by_ifname(const std::string&ifn)const;
};

struct ipv6_route{
	ipv6_cidr dest{};
	ipv6 next{};
	std::string ifname{};
	int metric=-1;
	ipv6_route();
	void clear();
	[[nodiscard]] int compare(const ipv6_route&other)const;
	[[nodiscard]] bool equals(const ipv6_route&other)const;
	[[nodiscard]] std::string to_string()const;
	[[nodiscard]] bool empty()const;
	[[nodiscard]] bool operator==(const ipv6_route&other)const;
	[[nodiscard]] bool operator!=(const ipv6_route&other)const;
	[[nodiscard]] bool operator<(const ipv6_route&other)const;
	[[nodiscard]] bool operator>(const ipv6_route&other)const;
	[[nodiscard]] bool operator<=(const ipv6_route&other)const;
	[[nodiscard]] bool operator>=(const ipv6_route&other)const;
};

struct ipv6_route_table{
	std::vector<ipv6_route>routes{};
	const ipv6_route&find(const ipv6&dest)const;
	ipv6_route&find(const ipv6&dest);
	ipv6_route_table by_ifname(const std::string&ifn)const;
};

struct resolv_conf{
	std::vector<std::string>nameservers{};
	std::string search{};
	std::vector<std::string>options{};
	void parse_line(const std::string&line);
	void parse_file(const std::string&content);
	void load_file(const std::string&path="/etc/resolv.conf");
	void save_file(const std::string&path="/etc/resolv.conf")const;
	void remove(const std::string&path="/etc/resolv.conf")const;
	std::string to_file()const;
};
#endif
