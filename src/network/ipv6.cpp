#include"net-utils.h"
#include"str-utils.h"
#include"error.h"
#include<cstring>

static ipv6 empty_ipv6{};

ipv6 ipv6::parse(const std::string&s){
	ipv6 m{};
	std::string str=s;
	if(auto pos=str.find('/');pos!=std::string::npos)
		str=str.substr(0, pos);
	if(str.empty()||str=="::")return empty_ipv6;
	if(str.starts_with("0x")){
		size_t idx=0;
		if(str.length()>34)throw InvalidArgument("invalid ipv6 address");
		m.d64[0]=std::stoull(str.substr(2,16),&idx,16);
		if(idx!=16)throw InvalidArgument("invalid ipv6 address");
		m.d64[1]=std::stoull(str.substr(18,16),&idx,16);
		if(idx!=16)throw InvalidArgument("invalid ipv6 address");
		return m;
	}
	auto items=str_split(str,':');
	int empty_count=0,nonempty_count=0;;
	for(auto&item:items)(item.empty()?empty_count:nonempty_count)++;
	if(items.size()>8+(empty_count>0?1:0))
		throw InvalidArgument("invalid ipv6 address");
	if(items.back().find('.')!=std::string::npos)
		nonempty_count++;
	int skip=-1;
	for(size_t i=0;i<items.size();i++)if(items[i].empty()){
		if(skip==-1)skip=i;
		else if(i!=skip+1)
			throw InvalidArgument("invalid ipv6 address");
	}
	uint16_t segs[8]={0};
	size_t idx=0;
	for(size_t i=0;i<items.size();i++){
		if(items[i].empty()){
			if(skip==(int)i)
				idx+=8-nonempty_count;
			continue;
		}
		if(idx>=8)throw InvalidArgument("invalid ipv6 address");
		if(items[i].find('.')!=std::string::npos){
			auto v4=ipv4::parse(items[i]);
			segs[idx++]=v4.d16[0];
			segs[idx++]=v4.d16[1];
			break;
		}
		size_t nidx=0;
		int val=std::stoi(items[i],&nidx,16);
		if(nidx==0||nidx>4||val<0||val>0xFFFF)
			throw InvalidArgument("invalid ipv6 address");
		segs[idx++]=htons(val);
	}
	if(idx!=8)throw InvalidArgument("invalid ipv6 address");
	memcpy(m.d16,segs,sizeof(segs));
	return m;
}

void ipv6::set(const socket_address&sa){
	if(sa.len<24)
		throw InvalidArgument("invalid socket address");
	if(sa.in4.sin_family!=AF_INET6)
		throw InvalidArgument("target is not ipv6 address");
	set(sa.in6.sin6_addr);
}

socket_address ipv6::to_socket_address()const{
	socket_address sa{};
	sa.len=sizeof(sockaddr_in6);
	sa.addr.sa_family=AF_INET6;
	sa.in6.sin6_addr=to_in6_addr();
	return sa;
}

in6_addr ipv6::to_in6_addr()const{
	in6_addr addr{};
	memcpy(&addr,d64,sizeof(d64));
	return addr;
}

ipv4 ipv6::to_ipv4()const{
	if(d64[0]==0&&d16[4]==0&&d16[5]==0xFFFF)
		return ipv4(d32[3]);
	throw InvalidArgument("not a ipv4-mapped ipv6 address");
}

std::string ipv6::to_string(bool upper)const{
	std::string ret{};
	if(d64[0]==0&&d16[4]==0&&d16[5]==0xFFFF){
		ret+=upper?"::FFFF:":"::ffff:";
		ret+=to_ipv4().to_string();
		return ret;
	}
	int zb=-1,zl=0;
	for(int i=0;i<8;)if(d16[i]==0){
		int j=i;
		while(j<8&&d16[j]==0)j++;
		if(j-i>zl)zb=i,zl=j-i;
		i=j;
	}else i++;
	if(zl<2)zb=-1;
	for(int i=0;i<8;)if(i==zb){
		ret+="::",i+=zl;
		if(i>=8)break;
	}else{
		if(!ret.empty()&&ret.back()!=':')ret+=":";
		ret+=ssprintf(upper?"%X":"%x",ntohs(d16[i++]));
	}
	return ret;
}

ipv6 ipv6::next(){
	ipv6 m=*this;
	m.d64[1]++;
	if(m.d64[1]==0)
		m.d64[0]++;
	return m;
}

ipv6 ipv6::prev(){
	ipv6 m=*this;
	m.d64[1]--;
	if(m.d64[1]==UINT64_MAX)
		m.d64[0]--;
	return m;
}

ipv6::ipv6(){clear();}
ipv6::ipv6(const std::string&n){set(n);}
ipv6::ipv6(const char*n){set(n);}
ipv6::ipv6(const sockaddr&sa){set(sa);}
ipv6::ipv6(const sockaddr*sa){set(sa);}
ipv6::ipv6(const sockaddr_in6&sa){set(sa);}
ipv6::ipv6(const sockaddr_in6*sa){set(sa);}
ipv6::ipv6(const socket_address&sa){set(sa);}
ipv6::ipv6(const socket_address*sa){set(sa);}
ipv6::ipv6(const in6_addr&ia){set(ia);}
ipv6::ipv6(const in6_addr*ia){set(ia);}
void ipv6::clear(){memset(d64,0,sizeof(d64));}
void ipv6::set(const ipv6&n){memcpy(d64,n.d64,sizeof(d64));}
void ipv6::set(const sockaddr&sa){set(socket_address{.len=24,.addr=sa});}
void ipv6::set(const sockaddr_in6&sa){set(socket_address{.len=24,.in6=sa});}
void ipv6::set(const std::string&n){if(n.empty())clear();else set(parse(n));}
void ipv6::set(const char*n){if(!n||!*n)clear();else set(parse(n));}
void ipv6::set(const sockaddr*sa){if(sa)set(*sa);else clear();}
void ipv6::set(const sockaddr_in6*sa){if(sa)set(*sa);else clear();}
void ipv6::set(const socket_address*sa){if(sa)set(*sa);else clear();}
void ipv6::set(const in6_addr*ia){if(ia)set(*ia);else clear();}
void ipv6::set(const in6_addr&ia){memcpy(d64,&ia,sizeof(d64));}
int ipv6::compare(const ipv6&other)const{return memcmp(d64,other.d64,sizeof(d64));}
bool ipv6::equals(const ipv6&other)const{return compare(other)==0;}
bool ipv6::empty()const{return equals(empty_ipv6);}
bool ipv6::operator==(const ipv6&other)const{return equals(other);}
bool ipv6::operator!=(const ipv6&other)const{return !equals(other);}
bool ipv6::operator<(const ipv6&other)const{return compare(other)<0;}
bool ipv6::operator>(const ipv6&other)const{return compare(other)>0;}
bool ipv6::operator<=(const ipv6&other)const{return compare(other)<=0;}
bool ipv6::operator>=(const ipv6&other)const{return compare(other)>=0;}
bool ipv6::operator==(const std::string&other)const{return equals(parse(other));}
bool ipv6::operator!=(const std::string&other)const{return !equals(parse(other));}
bool ipv6::operator<(const std::string&other)const{return compare(parse(other))<0;}
bool ipv6::operator>(const std::string&other)const{return compare(parse(other))>0;}
bool ipv6::operator<=(const std::string&other)const{return compare(parse(other))<=0;}
bool ipv6::operator>=(const std::string&other)const{return compare(parse(other))>=0;}
bool ipv6::operator==(const char*other)const{return equals(parse(other));}
bool ipv6::operator!=(const char*other)const{return !equals(parse(other));}
bool ipv6::operator<(const char*other)const{return compare(parse(other))<0;}
bool ipv6::operator>(const char*other)const{return compare(parse(other))>0;}
bool ipv6::operator<=(const char*other)const{return compare(parse(other))<=0;}
bool ipv6::operator>=(const char*other)const{return compare(parse(other))>=0;}
ipv6::operator std::string()const{return to_string();}
ipv6::operator in6_addr()const{return to_in6_addr();}
ipv6::operator sockaddr_in6()const{return to_socket_address().in6;}
ipv6::operator sockaddr()const{return to_socket_address().addr;}
ipv6::operator socket_address()const{return to_socket_address();}
ipv6::operator ipv4()const{return to_ipv4();}
