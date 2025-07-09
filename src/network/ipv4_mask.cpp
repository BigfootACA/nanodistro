#include"net-utils.h"
#include"error.h"
#include<cstring>

static ipv4_mask empty_ipv4_mask{};

ipv4_mask ipv4_mask::parse(const std::string&s){
	ipv4_mask m{};
	std::string str=s;
	if(s.find('.')==std::string::npos){
		size_t idx=0;
		m.prefix=std::stoi(str,&idx);
		if(idx!=str.length())throw InvalidArgument("invalid ipv4 mask");
		if(m.prefix<0||m.prefix>32)
			throw InvalidArgument("invalid ipv4 mask");
		return m;
	}else m.set(ipv4::parse(str));
	return m;
}

void ipv4_mask::set(const ipv4&v){
	if((v.v&(1<<31))==0&&(v.v&1)!=0)
		set_wildcard(v);
	else
		set_netmask(v);
}

socket_address ipv4_mask::to_socket_address()const{
	socket_address sa{};
	sa.len=sizeof(sockaddr_in);
	sa.addr.sa_family=AF_INET;
	sa.in4.sin_addr=to_in_addr();
	return sa;
}

void ipv4_mask::set_netmask(const ipv4&mask){
	prefix=0;
	bool zero_found=false;
	for(int i=0;i<32;i++){
		bool bit=mask.d[i/8]&(1<<(7-i%8));
		if(bit){
			if(zero_found)throw InvalidArgument("invalid ipv4 mask");
			prefix++;
		}else zero_found=true;
	}
}

void ipv4_mask::set_wildcard(const ipv4&mask){
	prefix=0;
	bool one_found=false;
	for(int i=0;i<32;i++){
		bool bit=mask.d[i/8]&(1<<(7-i%8));
		if(!bit){
			if(one_found)throw InvalidArgument("invalid ipv4 mask");
			prefix++;
		}else one_found=true;
	}
	
}

ipv4 ipv4_mask::to_netmask()const{
	if(prefix<0||prefix>32)throw InvalidArgument("invalid ipv4 mask");
	return ipv4((UINT32_MAX<<(32-prefix))&UINT32_MAX);
}

ipv4 ipv4_mask::to_wildcard()const{
	if(prefix<0||prefix>32)throw InvalidArgument("invalid ipv4 mask");
	return ipv4((UINT32_MAX>>prefix)&UINT32_MAX);
}

int ipv4_mask::compare(const ipv4_mask&other)const{
	if(prefix<other.prefix)return -1;
	if(prefix>other.prefix)return 1;
	if(prefix==other.prefix)return 0;
	throw InvalidArgument("invalid ipv4 mask");
}

void ipv4_mask::set(int prefix){
	if(prefix<0||prefix>32)throw InvalidArgument("invalid ipv4 mask");
	this->prefix=prefix;
}

ipv4_mask::ipv4_mask(){clear();}
ipv4_mask::ipv4_mask(const std::string&n){set(n);}
ipv4_mask::ipv4_mask(const char*n){set(n);}
ipv4_mask::ipv4_mask(int p){set(p);}
ipv4_mask::ipv4_mask(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4){set(d1,d2,d3,d4);}
void ipv4_mask::clear(){prefix=0;}
void ipv4_mask::set(const ipv4_mask&v){set(v.prefix);}
void ipv4_mask::set(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4){set_netmask(ipv4(d1,d2,d3,d4));}
void ipv4_mask::set(const std::string&n){if(n.empty())clear();else set(parse(n));}
void ipv4_mask::set(const char*n){if(!n||!*n)clear();else set(parse(n));}
in_addr ipv4_mask::to_in_addr()const{return to_netmask().to_in_addr();}
sockaddr_in ipv4_mask::to_sockaddr_in()const{return to_socket_address().in4;}
sockaddr ipv4_mask::to_sockaddr()const{return to_socket_address().addr;}
std::string ipv4_mask::to_string()const{return to_netmask().to_string();}
bool ipv4_mask::equals(const ipv4_mask&other)const{return compare(other)==0;}
bool ipv4_mask::empty()const{return equals(empty_ipv4_mask);}
bool ipv4_mask::operator==(const ipv4_mask&other)const{return equals(other);}
bool ipv4_mask::operator!=(const ipv4_mask&other)const{return !equals(other);}
bool ipv4_mask::operator<(const ipv4_mask&other)const{return compare(other)<0;}
bool ipv4_mask::operator>(const ipv4_mask&other)const{return compare(other)>0;}
bool ipv4_mask::operator<=(const ipv4_mask&other)const{return compare(other)<=0;}
bool ipv4_mask::operator>=(const ipv4_mask&other)const{return compare(other)>=0;}
bool ipv4_mask::operator==(const std::string&other)const{return equals(parse(other));}
bool ipv4_mask::operator!=(const std::string&other)const{return !equals(parse(other));}
bool ipv4_mask::operator<(const std::string&other)const{return compare(parse(other))<0;}
bool ipv4_mask::operator>(const std::string&other)const{return compare(parse(other))>0;}
bool ipv4_mask::operator<=(const std::string&other)const{return compare(parse(other))<=0;}
bool ipv4_mask::operator>=(const std::string&other)const{return compare(parse(other))>=0;}
bool ipv4_mask::operator==(const char*other)const{return equals(parse(other));}
bool ipv4_mask::operator!=(const char*other)const{return !equals(parse(other));}
bool ipv4_mask::operator<(const char*other)const{return compare(parse(other))<0;}
bool ipv4_mask::operator>(const char*other)const{return compare(parse(other))>0;}
bool ipv4_mask::operator<=(const char*other)const{return compare(parse(other))<=0;}
bool ipv4_mask::operator>=(const char*other)const{return compare(parse(other))>=0;}
ipv4_mask::operator std::string()const{return to_string();}
ipv4_mask::operator uint32_t()const{return prefix;}
ipv4_mask::operator in_addr()const{return to_in_addr();}
ipv4_mask::operator sockaddr_in()const{return to_sockaddr_in();}
ipv4_mask::operator sockaddr()const{return to_sockaddr();}
ipv4_mask::operator socket_address()const{return to_socket_address();}
