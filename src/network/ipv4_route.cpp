#include"net-utils.h"
#include"error.h"
#include<cstring>

static ipv4_route empty_ipv4_route{};

int ipv4_route::compare(const ipv4_route&other)const{
	if(auto r=dest.compare(other.dest);r!=0)return r;
	if(auto r=next.compare(other.next);r!=0)return r;
	return 0;
}

std::string ipv4_route::to_string()const{
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

const ipv4_route&ipv4_route_table::find(const ipv4&dest)const{
	const ipv4_route*best=nullptr;
	int bp=-1,bm=INT32_MAX;
	for(const auto&r:routes){
		int p=r.dest.mask.prefix;
		int m=(r.metric<0)?INT32_MAX:r.metric;
		if(!r.dest.is_addr_in(dest))continue;
		if(p>bp||(p==bp&&m<bm))
			best=&r,bp=p,bm=r.metric;
	}
	if(best)return*best;
	throw RuntimeError("ipv4 route for {} not found",dest.to_string());
}

ipv4_route&ipv4_route_table::find(const ipv4&dest){
	ipv4_route*best=nullptr;
	int bp=-1,bm=INT32_MAX;
	for(auto&r:routes){
		int p=r.dest.mask.prefix;
		int m=(r.metric<0)?INT32_MAX:r.metric;
		if(!r.dest.is_addr_in(dest))continue;
		if(p>bp||(p==bp&&m<bm))
			best=&r,bp=p,bm=r.metric;
	}
	if(best)return*best;
	errno=ENETUNREACH;
	throw ErrnoError("ipv4 route for {} not found",dest.to_string());
}

ipv4_route_table ipv4_route_table::by_ifname(const std::string&ifn)const{
	ipv4_route_table table{};
	for(auto&r:routes)
		if(r.ifname==ifn)
			table.routes.push_back(r);
	return table;
}

ipv4_route::ipv4_route(){clear();}
void ipv4_route::clear(){*this=empty_ipv4_route;}
bool ipv4_route::equals(const ipv4_route&other)const{return compare(other)==0;}
bool ipv4_route::empty()const{return equals(empty_ipv4_route);}
bool ipv4_route::operator==(const ipv4_route&other)const{return equals(other);}
bool ipv4_route::operator!=(const ipv4_route&other)const{return !equals(other);}
bool ipv4_route::operator<(const ipv4_route&other)const{return compare(other)<0;}
bool ipv4_route::operator>(const ipv4_route&other)const{return compare(other)>0;}
bool ipv4_route::operator<=(const ipv4_route&other)const{return compare(other)<=0;}
bool ipv4_route::operator>=(const ipv4_route&other)const{return compare(other)>=0;}
