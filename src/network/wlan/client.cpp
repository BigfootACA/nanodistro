#include"internal.h"
#include"str-utils.h"
#include"error.h"

void wlan_client_impl::terminate(){
	run("TERMINATE");
}

void wlan_client_impl::start_scan(){
	run("SCAN");
}

std::map<std::string,std::string>wlan_client_impl::get_status(){
	return parse_environ(exec("STATUS"));
}

static std::vector<std::string>parse_flags(const std::string&flags){
	if(!flags.starts_with('[')||!flags.ends_with(']'))return {};
	return str_split(flags.substr(1,flags.length()-2),"][");
}

std::list<std::shared_ptr<wlan_scan_result>>wlan_client_impl::scan_results(){
	std::list<std::shared_ptr<wlan_scan_result>>ret{};
	auto res=exec("SCAN_RESULTS");
	for(auto&line:str_split(res,'\n')){
		str_trim(line);
		if(line.empty())continue;
		if(line.starts_with("bssid / "))continue;
		auto items=str_split(line,'\t');
		if(items.size()<5)continue;
		auto r=std::make_shared<wlan_scan_result>();
		r->bssid=items[0];
		r->freq=std::stoi(items[1]);
		r->rssi=std::stoi(items[2]);
		r->flags=parse_flags(items[3]);
		r->ssid=str_trim_to(items[4]);
		ret.push_back(r);
	}
	return ret;
}

std::list<std::shared_ptr<wlan_network>>wlan_client_impl::list_networks(){
	std::list<std::shared_ptr<wlan_network>>ret{};
	auto res=exec("LIST_NETWORKS");
	for(auto&line:str_split(res,'\n')){
		str_trim(line);
		if(line.empty())continue;
		if(line.starts_with("network id / "))continue;
		auto items=str_split(line,'\t');
		if(items.size()<4)continue;
		wlan_network_item item{};
		item.id=std::stoi(items[0]);
		item.ssid=str_trim_to(items[1]);
		if(items[2]!="any")item.bssid=items[2];
		item.flags=parse_flags(items[3]);
		auto self=shared_from_this();
		ret.push_back(std::make_shared<wlan_network_impl>(self,item));
	}
	return ret;
}

std::shared_ptr<wlan_network>wlan_client_impl::add_network(){
	auto res=exec("ADD_NETWORK");
	if(res.empty()||res=="FAIL")
		throw RuntimeError("failed to add network");
	int nid=-1;
	try{
		size_t idx=0;
		str_trim(res);
		nid=std::stoi(res,&idx);
		if(idx!=res.length()||nid<0)
			throw std::invalid_argument("invalid network id");
	}catch(...){
		throw RuntimeError("failed to parse network id {}",res);
	}
	wlan_network_item item{};
	item.id=nid;
	item.flags.push_back("DISABLED");
	auto self=shared_from_this();
	return std::make_shared<wlan_network_impl>(self,item);
}
