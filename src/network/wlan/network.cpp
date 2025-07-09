#include"internal.h"
#include"error.h"

wlan_network::wlan_network(const wlan_network_item&item):item(item){}

wlan_network_impl::wlan_network_impl(
	const std::shared_ptr<wlan_client_impl>&clt,
	const wlan_network_item&item
):wlan_network(item),wifi(clt){}

static bool check(const std::string&v){
	bool quoted=v.starts_with('"')&&v.ends_with('"');
	for(size_t i=quoted;i<v.length()-quoted;i++)
		if(v[i]=='"'||std::isspace(v[i]))
			if(!quoted)return false;
	return true;
}

void wlan_network_impl::set_value(const std::string&var,const std::string&val){
	if(!check(var))throw InvalidArgument("invalid variable name {}",var);
	if(!check(val))throw InvalidArgument("invalid variable value {}",val);
	cur_wifi()->run(std::format("SET_NETWORK {} {} {}",item.id,var,val));	
}

std::string wlan_network_impl::get_value(const std::string&var){
	if(!check(var))throw InvalidArgument("invalid variable name {}",var);
	return cur_wifi()->exec(std::format("GET_NETWORK {} {}",item.id,var));
}

void wlan_network_impl::select(){
	cur_wifi()->run(std::format("SELECT_NETWORK {}",item.id));
}

void wlan_network_impl::enable(){
	cur_wifi()->run(std::format("ENABLE_NETWORK {}",item.id));
}

void wlan_network_impl::disable(){
	cur_wifi()->run(std::format("DISABLE_NETWORK {}",item.id));
}

void wlan_network_impl::remove(){
	cur_wifi()->run(std::format("REMOVE_NETWORK {}",item.id));
}

void wlan_network::set_str_value(const std::string&var,const std::string&val){
	set_value(var,std::format("\"{}\"",val));
}

std::shared_ptr<wlan_client_impl>wlan_network_impl::cur_wifi(){
	if(auto clt=wifi.lock())return clt;
	throw RuntimeError("wlan client not opened");
}
