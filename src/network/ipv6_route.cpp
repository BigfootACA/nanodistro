#include"net-utils.h"
#include"error.h"
#include<cstring>

static ipv6_route empty_ipv6_route{};

int ipv6_route::compare(const ipv6_route&other)const{
	if(auto r=dest.compare(other.dest);r!=0)return r;
	if(auto r=next.compare(other.next);r!=0)return r;
	return 0;
}

std::string ipv6_route::to_string()const{
	std::string ret{};
	if(dest.empty())ret+="default";
	else ret+=dest.to_string();
	if(!next.empty())
		ret+=std::format(" via {}",next.to_string());
	if(!ifname.empty()){
		if(!ret.empty())ret+=' ';
		ret+=std::format("dev {}",ifname);
	}
	if(metric>=0){
		ret+=' ';
		ret+=std::format("metric {}",metric);
	}
	return ret;
}

const ipv6_route&ipv6_route_table::find(const ipv6&dest)const{
	const ipv6_route*best=nullptr;
	int bp=-1,bm=INT32_MAX;
	for(const auto&r:routes){
		int p=r.dest.prefix;
		int m=(r.metric<0)?INT32_MAX:r.metric;
		if(!r.dest.is_addr_in(dest))continue;
		if(p>bp||(p==bp&&m<bm))
			best=&r,bp=p,bm=r.metric;
	}
	if(best)return*best;
	throw RuntimeError("ipv6 route for {} not found",dest.to_string());
}

ipv6_route&ipv6_route_table::find(const ipv6&dest){
	ipv6_route*best=nullptr;
	int bp=-1,bm=INT32_MAX;
	for(auto&r:routes){
		int p=r.dest.prefix;
		int m=(r.metric<0)?INT32_MAX:r.metric;
		if(!r.dest.is_addr_in(dest))continue;
		if(p>bp||(p==bp&&m<bm))
			best=&r,bp=p,bm=r.metric;
	}
	if(best)return*best;
	errno=ENETUNREACH;
	throw ErrnoError("ipv4 route for {} not found",dest.to_string());
}

ipv6_route_table ipv6_route_table::by_ifname(const std::string&ifn)const{
	ipv6_route_table table{};
	for(auto&r:routes)
		if(r.ifname==ifn)
			table.routes.push_back(r);
	return table;
}

ipv6_route::ipv6_route(){clear();}
void ipv6_route::clear(){*this=empty_ipv6_route;}
bool ipv6_route::equals(const ipv6_route&other)const{return compare(other)==0;}
bool ipv6_route::empty()const{return equals(empty_ipv6_route);}
bool ipv6_route::operator==(const ipv6_route&other)const{return equals(other);}
bool ipv6_route::operator!=(const ipv6_route&other)const{return !equals(other);}
bool ipv6_route::operator<(const ipv6_route&other)const{return compare(other)<0;}
bool ipv6_route::operator>(const ipv6_route&other)const{return compare(other)>0;}
bool ipv6_route::operator<=(const ipv6_route&other)const{return compare(other)<=0;}
bool ipv6_route::operator>=(const ipv6_route&other)const{return compare(other)>=0;}
