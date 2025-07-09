#include"net-utils.h"
#include"str-utils.h"
#include"error.h"
#include<cstring>
#include<net/if_arp.h>

static mac empty_mac{};

mac mac::parse(const std::string&str){
	mac m{};
	size_t off=0,i;
	int data[sizeof(m.d)*2]{};
	if(str.empty())return empty_mac;
	for(i=0;i<str.length()&&off<sizeof(data);i++){
		if(str[i]==':'||str[i]=='-'){
			if(off>0&&(off%2==0))continue;
			else throw InvalidArgument("invalid mac address");
		}
		data[off++]=hex2dec(str[i]);
	}
	if(i!=str.length()||off!=sizeof(m.d)*2)
		throw InvalidArgument("invalid mac address");
	for(i=0;i<sizeof(m.d);i++)
		m.d[i]=data[i*2]<<4|data[i*2+1];
	return m;
}

void mac::set(const socket_address&sa){
	if(sa.len<8)
		throw InvalidArgument("invalid socket address");
	if(sa.addr.sa_family!=ARPHRD_ETHER)
		throw InvalidArgument("target is not mac address");
	memcpy(d,sa.addr.sa_data,sizeof(d));
}

std::string mac::to_string(const std::string&sep,bool upper)const{
	std::string ret{};
	for(size_t i=0;i<sizeof(d);i++){
		if(i>0)ret+=sep;
		ret+=dec2hex(d[i]&0xff,upper);
		ret+=dec2hex(d[i]>>4&0xff,upper);
	}
	return ret;
}

mac::mac(){clear();}
mac::mac(const std::string&n){set(n);}
mac::mac(const char*n){set(n);}
mac::mac(const sockaddr*n){set(n);}
mac::mac(const sockaddr&n){set(n);}
mac::mac(const socket_address*n){set(n);}
mac::mac(const socket_address&n){set(n);}
void mac::clear(){memset(d,0,sizeof(d));}
void mac::set(const mac&n){memcpy(d,n.d,sizeof(d));}
void mac::set(const sockaddr&sa){set(socket_address{.len=8,.addr=sa});}
void mac::set(const std::string&n){if(n.empty())clear();else set(parse(n));}
void mac::set(const char*n){if(!n||!*n)clear();else set(parse(n));}
void mac::set(const sockaddr*sa){if(sa)set(*sa);else clear();}
void mac::set(const socket_address*sa){if(sa)set(*sa);else clear();}
int mac::compare(const mac&other)const{return memcmp(d,other.d,sizeof(d));}
bool mac::equals(const mac&other)const{return compare(other)==0;}
bool mac::empty()const{return equals(empty_mac);}
bool mac::operator==(const mac&other)const{return equals(other);}
bool mac::operator!=(const mac&other)const{return !equals(other);}
bool mac::operator<(const mac&other)const{return compare(other)<0;}
bool mac::operator>(const mac&other)const{return compare(other)>0;}
bool mac::operator<=(const mac&other)const{return compare(other)<=0;}
bool mac::operator>=(const mac&other)const{return compare(other)>=0;}
bool mac::operator==(const std::string&other)const{return equals(parse(other));}
bool mac::operator!=(const std::string&other)const{return !equals(parse(other));}
bool mac::operator<(const std::string&other)const{return compare(parse(other))<0;}
bool mac::operator>(const std::string&other)const{return compare(parse(other))>0;}
bool mac::operator<=(const std::string&other)const{return compare(parse(other))<=0;}
bool mac::operator>=(const std::string&other)const{return compare(parse(other))>=0;}
bool mac::operator==(const char*other)const{return equals(parse(other));}
bool mac::operator!=(const char*other)const{return !equals(parse(other));}
bool mac::operator<(const char*other)const{return compare(parse(other))<0;}
bool mac::operator>(const char*other)const{return compare(parse(other))>0;}
bool mac::operator<=(const char*other)const{return compare(parse(other))<=0;}
bool mac::operator>=(const char*other)const{return compare(parse(other))>=0;}
mac::operator std::string()const{return to_string();}
