#ifndef WLAN_H
#define WLAN_H
#include<map>
#include<list>
#include<string>
#include<vector>
#include<memory>
#include<cstdint>
#include<functional>
#include<sys/socket.h>
#include"net-utils.h"

struct wlan_scan_result{
	mac bssid;
	uint32_t freq;
	int32_t rssi;
	std::vector<std::string>flags;
	std::string ssid;
	bool is_flag(const std::string&flag)const;
	bool is_flags(const std::vector<std::string>&flags)const;
	bool is_flag_contains(const std::string&flag)const;
	bool is_flags_contains(const std::vector<std::string>&flags)const;
};

struct wlan_network_item{
	uint32_t id;
	std::string ssid;
	mac bssid;
	std::vector<std::string>flags;
	bool is_flag(const std::string&flag)const;
	bool is_flags(const std::vector<std::string>&flags)const;
	bool is_flag_contains(const std::string&flag)const;
	bool is_flags_contains(const std::vector<std::string>&flags)const;
};

class wlan_network{
	public:
		inline wlan_network(const wlan_network_item&item);
		virtual ~wlan_network()=default;
		virtual void set_value(const std::string&var,const std::string&val)=0;
		void set_str_value(const std::string&var,const std::string&val);
		virtual std::string get_value(const std::string&var)=0;
		virtual void select()=0;
		virtual void enable()=0;
		virtual void disable()=0;
		virtual void remove()=0;
		const wlan_network_item item;
};

enum wpa_states{
	WPA_DISCONNECTED,
	WPA_INTERFACE_DISABLED,
	WPA_INACTIVE,
	WPA_SCANNING,
	WPA_AUTHENTICATING,
	WPA_ASSOCIATING,
	WPA_ASSOCIATED,
	WPA_4WAY_HANDSHAKE,
	WPA_GROUP_HANDSHAKE,
	WPA_COMPLETED,
	WPA_UNKNOWN,
};

class wlan_client{
	public:
	virtual ~wlan_client()=default;
	virtual void close()=0;
	virtual void terminate()=0;
	virtual void start_scan()=0;
	virtual wpa_states get_wpa_state();
	virtual std::string get_status(const std::string&key);
	virtual std::map<std::string,std::string>get_status()=0;
	virtual std::list<std::shared_ptr<wlan_scan_result>>scan_results()=0;
	virtual std::list<std::shared_ptr<wlan_network>>list_networks()=0;
	virtual std::shared_ptr<wlan_network>add_network()=0;
	virtual uint64_t listen_event(const std::function<void(uint64_t id,const std::string&)>&cb)=0;
	virtual void unlisten_event(uint64_t id)=0;
	static std::list<std::string>list_wlan_devices();
	static std::list<std::string>list_wlan_sockets();
	static std::shared_ptr<wlan_client>create(const std::string&dev);
};

extern wpa_states string_to_wpa_states(const std::string&state);

#endif
