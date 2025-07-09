#include"net-utils.h"
#include"error.h"
#include<cstring>

static ipv6_cidr empty_ipv6_cidr{};

ipv6_cidr ipv6_cidr::parse(const std::string&s){
	ipv6_cidr m{};
	std::string str=s;
	auto pos=str.find('/');
	if(pos==std::string::npos)
		throw InvalidArgument("invalid ipv6 cidr");
	m.addr=str.substr(0,pos);
	size_t idx=0;
	auto p=str.substr(pos+1);
	m.prefix=std::stoi(p,&idx,10);
	if(idx!=p.length()||m.prefix<0||m.prefix>128)
		throw InvalidArgument("invalid ipv6 prefix");
	return m;
}

socket_address ipv6_cidr::to_socket_address()const{
	socket_address sa{};
	sa.len=sizeof(sockaddr_in6);
	sa.addr.sa_family=AF_INET6;
	sa.in6.sin6_addr=to_in6_addr();
	return sa;
}

ipv6 ipv6_cidr::brd()const{
	ipv6 m=addr;
	if(prefix<0||prefix>128)
		throw InvalidArgument("invalid ipv6 prefix");
	else if(prefix==0){
		m.d64[0]=UINT64_MAX;
		m.d64[1]=UINT64_MAX;
	}else if(prefix<64){
		m.d64[0]&=(UINT64_MAX<<(64-prefix));
		m.d64[0]|=(UINT64_MAX>>prefix);
		m.d64[1]=UINT64_MAX;
	}else if(prefix==64){
		m.d64[1]=UINT64_MAX;
	}else if(prefix<128){
		m.d64[1]|=(UINT64_MAX>>(prefix-64));
	}
	return m;
}

ipv6 ipv6_cidr::net()const{
	ipv6 m=addr;
	if(prefix<0||prefix>128)
		throw InvalidArgument("invalid ipv6 prefix");
	else if(prefix==0){
		m.d64[0]=0;
		m.d64[1]=0;
	}else if(prefix<64){
		m.d64[0]&=UINT64_MAX<<(64-prefix);
		m.d64[1]=0;
	}else if(prefix==64){
		m.d64[1]=0;
	}else if(prefix>64&&prefix<128){
		m.d64[1]&=(UINT64_MAX<<(128-prefix));
	}
	return m;
}

int ipv6_cidr::compare(const ipv6_cidr&other)const{
	if(auto r=addr.compare(other.addr);r!=0)return r;
	if(prefix<other.prefix)return -1;
	if(prefix>other.prefix)return 1;
	if(prefix==other.prefix)return 0;
	throw InvalidArgument("invalid ipv6 prefix");
}

void ipv6_cidr::set(const ipv6&addr,int prefix){
	if(prefix<0||prefix>128)
		throw InvalidArgument("invalid ipv6 prefix");
	this->addr.set(addr);
	this->prefix=prefix;
}
ipv6 ipv6_cidr::begin()const{
	if(prefix==0)return {};
	if(prefix==128)
		return addr;
	return net().next();
}

ipv6 ipv6_cidr::end()const{
	if(prefix==0){
		ipv6 m;
		m.d64[0]=UINT64_MAX;
		m.d64[1]=UINT64_MAX;
		return m;
	}
	if(prefix==128)
		return addr;
	return brd().prev();
}

ipv6_cidr::ipv6_cidr(){clear();}
ipv6_cidr::ipv6_cidr(const ipv6&addr,int prefix){set(addr,prefix);}
ipv6_cidr::ipv6_cidr(const std::string&n){set(n);}
ipv6_cidr::ipv6_cidr(const char*n){set(n);}
void ipv6_cidr::clear(){addr.clear();prefix=0;}
void ipv6_cidr::set(const std::string&n){if(n.empty())clear();else set(parse(n));}
void ipv6_cidr::set(const char*n){if(!n||!*n)clear();else set(parse(n));}
in6_addr ipv6_cidr::to_in6_addr()const{return addr.to_in6_addr();}
sockaddr_in6 ipv6_cidr::to_sockaddr_in6()const{return to_socket_address().in6;}
sockaddr ipv6_cidr::to_sockaddr()const{return to_socket_address().addr;}
std::string ipv6_cidr::to_string()const{return std::format("{}/{}",addr.to_string(),prefix);}
bool ipv6_cidr::is_addr_in(const ipv6&addr)const{return addr>=begin()&&addr<=end();}
bool ipv6_cidr::equals(const ipv6_cidr&other)const{return compare(other)==0;}
bool ipv6_cidr::empty()const{return equals(empty_ipv6_cidr);}
bool ipv6_cidr::operator==(const ipv6_cidr&other)const{return equals(other);}
bool ipv6_cidr::operator!=(const ipv6_cidr&other)const{return !equals(other);}
bool ipv6_cidr::operator<(const ipv6_cidr&other)const{return compare(other)<0;}
bool ipv6_cidr::operator>(const ipv6_cidr&other)const{return compare(other)>0;}
bool ipv6_cidr::operator<=(const ipv6_cidr&other)const{return compare(other)<=0;}
bool ipv6_cidr::operator>=(const ipv6_cidr&other)const{return compare(other)>=0;}
bool ipv6_cidr::operator==(const std::string&other)const{return equals(parse(other));}
bool ipv6_cidr::operator!=(const std::string&other)const{return !equals(parse(other));}
bool ipv6_cidr::operator<(const std::string&other)const{return compare(parse(other))<0;}
bool ipv6_cidr::operator>(const std::string&other)const{return compare(parse(other))>0;}
bool ipv6_cidr::operator<=(const std::string&other)const{return compare(parse(other))<=0;}
bool ipv6_cidr::operator>=(const std::string&other)const{return compare(parse(other))>=0;}
bool ipv6_cidr::operator==(const char*other)const{return equals(parse(other));}
bool ipv6_cidr::operator!=(const char*other)const{return !equals(parse(other));}
bool ipv6_cidr::operator<(const char*other)const{return compare(parse(other))<0;}
bool ipv6_cidr::operator>(const char*other)const{return compare(parse(other))>0;}
bool ipv6_cidr::operator<=(const char*other)const{return compare(parse(other))<=0;}
bool ipv6_cidr::operator>=(const char*other)const{return compare(parse(other))>=0;}
ipv6_cidr::operator std::string()const{return to_string();}
ipv6_cidr::operator in6_addr()const{return to_in6_addr();}
ipv6_cidr::operator sockaddr_in6()const{return to_sockaddr_in6();}
ipv6_cidr::operator sockaddr()const{return to_sockaddr();}
ipv6_cidr::operator socket_address()const{return to_socket_address();}
