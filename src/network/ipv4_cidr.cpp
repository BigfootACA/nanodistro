#include"net-utils.h"
#include"error.h"
#include<cstring>

static ipv4_cidr empty_ipv4_cidr{};

ipv4_cidr ipv4_cidr::parse(const std::string&s){
	ipv4_cidr m{};
	std::string str=s;
	auto pos=str.find('/');
	if(pos==std::string::npos)
		throw InvalidArgument("invalid ipv4 cidr");
	m.addr=str.substr(0,pos);
	m.mask=str.substr(pos+1);
	return m;
}

socket_address ipv4_cidr::to_socket_address()const{
	socket_address sa{};
	sa.len=sizeof(sockaddr_in);
	sa.addr.sa_family=AF_INET;
	sa.in4.sin_addr=to_in_addr();
	return sa;
}

ipv4 ipv4_cidr::begin()const{
	if(mask.prefix>0&&mask.prefix<32)
		return net().next();
	return addr;
}

ipv4 ipv4_cidr::end()const{
	if(mask.prefix>0&&mask.prefix<32)
		return brd().prev();
	return addr;
}

int ipv4_cidr::compare(const ipv4_cidr&other)const{
	if(auto r=addr.compare(other.addr);r!=0)return r;
	if(auto r=mask.compare(other.mask);r!=0)return r;
	return 0;
}

ipv4_cidr::ipv4_cidr(){clear();}
ipv4_cidr::ipv4_cidr(const ipv4&addr,int prefix){set(addr,prefix);}
ipv4_cidr::ipv4_cidr(const std::string&n){set(n);}
ipv4_cidr::ipv4_cidr(const char*n){set(n);}
ipv4 ipv4_cidr::net()const{return ipv4(addr.v&mask.to_netmask().v);}
ipv4 ipv4_cidr::brd()const{return net()+mask.to_wildcard();}
void ipv4_cidr::clear(){addr.clear();mask.clear();}
void ipv4_cidr::set(const std::string&n){if(n.empty())clear();else set(parse(n));}
void ipv4_cidr::set(const char*n){if(!n||!*n)clear();else set(parse(n));}
void ipv4_cidr::set(const ipv4&addr,int prefix){this->addr.set(addr);this->mask.set(prefix);}
in_addr ipv4_cidr::to_in_addr()const{return addr.to_in_addr();}
sockaddr_in ipv4_cidr::to_sockaddr_in()const{return to_socket_address().in4;}
sockaddr ipv4_cidr::to_sockaddr()const{return to_socket_address().addr;}
std::string ipv4_cidr::to_string()const{return std::format("{}/{}",addr.to_string(),mask.prefix);}
bool ipv4_cidr::is_addr_in(const ipv4&addr)const{return addr>=begin()&&addr<=end();}
bool ipv4_cidr::equals(const ipv4_cidr&other)const{return compare(other)==0;}
bool ipv4_cidr::empty()const{return equals(empty_ipv4_cidr);}
bool ipv4_cidr::operator==(const ipv4_cidr&other)const{return equals(other);}
bool ipv4_cidr::operator!=(const ipv4_cidr&other)const{return !equals(other);}
bool ipv4_cidr::operator<(const ipv4_cidr&other)const{return compare(other)<0;}
bool ipv4_cidr::operator>(const ipv4_cidr&other)const{return compare(other)>0;}
bool ipv4_cidr::operator<=(const ipv4_cidr&other)const{return compare(other)<=0;}
bool ipv4_cidr::operator>=(const ipv4_cidr&other)const{return compare(other)>=0;}
bool ipv4_cidr::operator==(const std::string&other)const{return equals(parse(other));}
bool ipv4_cidr::operator!=(const std::string&other)const{return !equals(parse(other));}
bool ipv4_cidr::operator<(const std::string&other)const{return compare(parse(other))<0;}
bool ipv4_cidr::operator>(const std::string&other)const{return compare(parse(other))>0;}
bool ipv4_cidr::operator<=(const std::string&other)const{return compare(parse(other))<=0;}
bool ipv4_cidr::operator>=(const std::string&other)const{return compare(parse(other))>=0;}
bool ipv4_cidr::operator==(const char*other)const{return equals(parse(other));}
bool ipv4_cidr::operator!=(const char*other)const{return !equals(parse(other));}
bool ipv4_cidr::operator<(const char*other)const{return compare(parse(other))<0;}
bool ipv4_cidr::operator>(const char*other)const{return compare(parse(other))>0;}
bool ipv4_cidr::operator<=(const char*other)const{return compare(parse(other))<=0;}
bool ipv4_cidr::operator>=(const char*other)const{return compare(parse(other))>=0;}
ipv4_cidr::operator std::string()const{return to_string();}
ipv4_cidr::operator in_addr()const{return to_in_addr();}
ipv4_cidr::operator sockaddr_in()const{return to_sockaddr_in();}
ipv4_cidr::operator sockaddr()const{return to_sockaddr();}
ipv4_cidr::operator socket_address()const{return to_socket_address();}
