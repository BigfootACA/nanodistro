#include"net-utils.h"
#include"str-utils.h"
#include"error.h"
#include<cstring>

static ipv4 empty_ipv4{};

ipv4 ipv4::parse(const std::string&s){
	ipv4 m{};
	std::string str=s;
	if(auto pos=str.find('/');pos!=std::string::npos)
		str=str.substr(0,pos);
	if(str.empty())return empty_ipv4;
	if(str.starts_with("0x")){
		size_t idx=0;
		if(str.length()>10)throw InvalidArgument("invalid ipv4 address");
		m.v=std::stoul(str,&idx,16);
		if(idx!=str.length())throw InvalidArgument("invalid ipv4 address");
		return m;
	}
	if(str.find('.')==std::string::npos){
		size_t idx=0;
		m.v=std::stoi(str,&idx);
		if(idx!=str.length())throw InvalidArgument("invalid ipv4 address");
		return m;
	}
	auto items=str_split(str,'.');
	auto il=items.size();
	if(il>4)throw InvalidArgument("invalid ipv4 address");
	for(size_t i=0;i<il;i++){
		if(items[i].empty())throw InvalidArgument("invalid ipv4 address");
		int r=std::stoi(items[i]);
		if(r<0||r>255)throw InvalidArgument("invalid ipv4 address");
		m.d[i]=r;
	}
	if(il==2)m.d[3]=m.d[1],m.d[1]=0;
	if(il==3)m.d[3]=m.d[2],m.d[2]=0;
	return m;
}

void ipv4::set(const socket_address&sa){
	if(sa.len<8)
		throw InvalidArgument("invalid socket address");
	if(sa.in4.sin_family!=AF_INET)
		throw InvalidArgument("target is not ipv4 address");
	set(sa.in4.sin_addr);
}

socket_address ipv4::to_socket_address()const{
	socket_address sa{};
	sa.len=sizeof(sockaddr_in);
	sa.addr.sa_family=AF_INET;
	sa.in4.sin_addr=to_in_addr();
	return sa;
}

std::string ipv4::to_string()const{
	return std::format(
		"{}.{}.{}.{}",
		(int)d[0],(int)d[1],(int)d[2],(int)d[3]
	);
}

ipv4::ipv4(){clear();}
ipv4::ipv4(const std::string&n){set(n);}
ipv4::ipv4(const char*n){set(n);}
ipv4::ipv4(uint32_t ip){set(ip);}
ipv4::ipv4(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4){set(d1,d2,d3,d4);}
ipv4::ipv4(const sockaddr&sa){set(sa);}
ipv4::ipv4(const sockaddr*sa){set(sa);}
ipv4::ipv4(const sockaddr_in&sa){set(sa);}
ipv4::ipv4(const sockaddr_in*sa){set(sa);}
ipv4::ipv4(const socket_address&sa){set(sa);}
ipv4::ipv4(const socket_address*sa){set(sa);}
ipv4::ipv4(const in_addr&ia){set(ia);}
ipv4::ipv4(const in_addr*ia){set(ia);}
ipv4 ipv4::next(){return ipv4(v+1);}
ipv4 ipv4::prev(){return ipv4(v-1);}
void ipv4::clear(){memset(d,0,sizeof(d));}
void ipv4::set(const ipv4&n){v=n.v;}
void ipv4::set(uint32_t ip){v=ip;}
void ipv4::set(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4){d[0]=d1;d[1]=d2;d[2]=d3;d[3]=d4;}
void ipv4::set(const sockaddr&sa){set(socket_address{.len=8,.addr=sa});}
void ipv4::set(const sockaddr_in&sa){set(socket_address{.len=8,.in4=sa});}
void ipv4::set(const std::string&n){if(n.empty())clear();else set(parse(n));}
void ipv4::set(const char*n){if(!n||!*n)clear();else set(parse(n));}
void ipv4::set(const sockaddr*sa){if(sa)set(*sa);else clear();}
void ipv4::set(const sockaddr_in*sa){if(sa)set(*sa);else clear();}
void ipv4::set(const socket_address*sa){if(sa)set(*sa);else clear();}
void ipv4::set(const in_addr*ia){if(ia)set(*ia);else clear();}
void ipv4::set(const in_addr&ia){v=ia.s_addr;}
in_addr ipv4::to_in_addr()const{return in_addr{.s_addr=v};}
sockaddr_in ipv4::to_sockaddr_in()const{return to_socket_address().in4;}
sockaddr ipv4::to_sockaddr()const{return to_socket_address().addr;}
int ipv4::compare(const ipv4&other)const{return memcmp(d,other.d,sizeof(d));}
bool ipv4::equals(const ipv4&other)const{return compare(other)==0;}
bool ipv4::empty()const{return equals(empty_ipv4);}
bool ipv4::operator==(const ipv4&other)const{return equals(other);}
bool ipv4::operator!=(const ipv4&other)const{return !equals(other);}
bool ipv4::operator<(const ipv4&other)const{return compare(other)<0;}
bool ipv4::operator>(const ipv4&other)const{return compare(other)>0;}
bool ipv4::operator<=(const ipv4&other)const{return compare(other)<=0;}
bool ipv4::operator>=(const ipv4&other)const{return compare(other)>=0;}
bool ipv4::operator==(const std::string&other)const{return equals(parse(other));}
bool ipv4::operator!=(const std::string&other)const{return !equals(parse(other));}
bool ipv4::operator<(const std::string&other)const{return compare(parse(other))<0;}
bool ipv4::operator>(const std::string&other)const{return compare(parse(other))>0;}
bool ipv4::operator<=(const std::string&other)const{return compare(parse(other))<=0;}
bool ipv4::operator>=(const std::string&other)const{return compare(parse(other))>=0;}
bool ipv4::operator==(const char*other)const{return equals(parse(other));}
bool ipv4::operator!=(const char*other)const{return !equals(parse(other));}
bool ipv4::operator<(const char*other)const{return compare(parse(other))<0;}
bool ipv4::operator>(const char*other)const{return compare(parse(other))>0;}
bool ipv4::operator<=(const char*other)const{return compare(parse(other))<=0;}
bool ipv4::operator>=(const char*other)const{return compare(parse(other))>=0;}
ipv4::operator std::string()const{return to_string();}
ipv4::operator uint32_t()const{return v;}
ipv4::operator in_addr()const{return to_in_addr();}
ipv4::operator sockaddr_in()const{return to_sockaddr_in();}
ipv4::operator sockaddr()const{return to_sockaddr();}
ipv4::operator socket_address()const{return to_socket_address();}
