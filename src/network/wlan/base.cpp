#include"log.h"
#include"configs.h"
#include"internal.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"std-utils.h"
#include<cstring>
#include<algorithm>
#include<functional>
#include<dirent.h>

bool wlan_scan_result::is_flag(const std::string&flag)const{
	return std::find(flags.begin(),flags.end(),flag)!=flags.end();
}

bool wlan_scan_result::is_flags(const std::vector<std::string>&flags)const{
	auto f=std::bind(&wlan_scan_result::is_flag,this,std::placeholders::_1);
	return std::any_of(flags.begin(),flags.end(),f);
}

bool wlan_scan_result::is_flag_contains(const std::string&flag)const{
	auto f=std::bind(str_contains,std::placeholders::_1,flag);
	return std::any_of(flags.begin(),flags.end(),f);
}

bool wlan_scan_result::is_flags_contains(const std::vector<std::string>&flags)const{
	auto f=std::bind(&wlan_scan_result::is_flag_contains,this,std::placeholders::_1);
	return std::any_of(flags.begin(),flags.end(),f);
}

bool wlan_network_item::is_flag(const std::string&flag)const{
	return std::find(flags.begin(),flags.end(),flag)!=flags.end();
}

bool wlan_network_item::is_flags(const std::vector<std::string>&flags)const{
	auto f=std::bind(&wlan_network_item::is_flag,this,std::placeholders::_1);
	return std::any_of(flags.begin(),flags.end(),f);
}

bool wlan_network_item::is_flag_contains(const std::string&flag)const{
	auto f=std::bind(str_contains,std::placeholders::_1,flag);
	return std::any_of(flags.begin(),flags.end(),f);
}

bool wlan_network_item::is_flags_contains(const std::vector<std::string>&flags)const{
	auto f=std::bind(&wlan_network_item::is_flag_contains,this,std::placeholders::_1);
	return std::any_of(flags.begin(),flags.end(),f);
}

std::list<std::string>wlan_client::list_wlan_sockets(){
	std::list<std::string>devs{};
	auto dir=get_wpa_supplicant_dir();
	DIR*d=opendir(dir.c_str());
	if(!d){
		log_warning("no wpa_supplicant dir {} found",dir);
		return {};
	}
	while(auto e=readdir(d)){
		if(!e||e->d_name[0]=='.'||e->d_type!=DT_SOCK)continue;
		log_debug("found wpa_supplicant socket {}",e->d_name);
		devs.push_back(e->d_name);
	}
	log_debug("found {} wpa_supplicant sockets",devs.size());
	closedir(d);
	return devs;
}

std::list<std::string>wlan_client::list_wlan_devices(){
	std::list<std::string>devs{};
	std::vector<std::string>allowed{};
	if(auto v=config["nanodistro"]["network"]["wlan"]["allowed"];v&&v.IsSequence())for(auto i:v)
		allowed.push_back(i.as<std::string>());
	DIR*d=opendir("/sys/class/net");
	if(!d)return {};
	while(auto e=readdir(d)){
		if(e->d_name[0]=='.')continue;
		if(e->d_type!=DT_LNK)continue;
		std::string name=e->d_name;
		if(name.starts_with("p2p"))continue;
		auto path=std::format("/sys/class/net/{}/phy80211",name);
		if(!fs_exists(path))continue;
		if(!allowed.empty()&&!std_contains(allowed,name))continue;
		log_debug("found wlan device {}",name);
		devs.push_back(name);
	}
	log_debug("found {} wlan devices",devs.size());
	closedir(d);
	return devs;
}

wpa_states string_to_wpa_states(const std::string&state){
	if(state=="DISCONNECTED")return WPA_DISCONNECTED;
	if(state=="INACTIVE")return WPA_INACTIVE;
	if(state=="INTERFACE_DISABLED")return WPA_INTERFACE_DISABLED;
	if(state=="SCANNING")return WPA_SCANNING;
	if(state=="AUTHENTICATING")return WPA_AUTHENTICATING;
	if(state=="ASSOCIATING")return WPA_ASSOCIATING;
	if(state=="ASSOCIATED")return WPA_ASSOCIATED;
	if(state=="4WAY_HANDSHAKE")return WPA_4WAY_HANDSHAKE;
	if(state=="GROUP_HANDSHAKE")return WPA_GROUP_HANDSHAKE;
	if(state=="COMPLETED")return WPA_COMPLETED;
	return WPA_UNKNOWN;
}

wpa_states wlan_client::get_wpa_state(){
	return string_to_wpa_states(get_status("wpa_state"));
}

std::string wlan_client::get_status(const std::string&key){
	auto m=get_status();
	auto it=m.find(key);
	if(it==m.end())return "";
	return it->second;
}
