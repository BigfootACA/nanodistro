#ifndef UI_NETWORK_H
#define UI_NETWORK_H
#include"ui.h"
#include"wlan.h"
#include<lvgl.h>
#include<list>
#include<memory>
#include<atomic>
class ui_draw_network;

class device_wifi{
	public:
		std::string name;
		std::shared_ptr<wlan_client>client=nullptr;
		lv_obj_t*button=nullptr;
		wpa_states state=WPA_UNKNOWN;
		mac cur_bssid{};
		std::map<std::string,std::string>status{};
		std::list<std::shared_ptr<wlan_scan_result>>results{};
		std::list<std::shared_ptr<wlan_network>>networks{};
};

class device_wifi_ssid{
	public:
		std::shared_ptr<device_wifi>wifi=nullptr;
		std::shared_ptr<wlan_scan_result>result=nullptr;
		std::shared_ptr<wlan_network>network=nullptr;
		enum connect_status{
			DISCONNECTED,
			CONNECTED_CURRENT,
			CONNECTED_OTHER,
		}status;
};

class device_eth{
	public:
		std::string name;
		lv_obj_t*button=nullptr;
};

enum ipv4_mode{
	IPV4_DHCP,
	IPV4_STATIC,
	IPV4_DISABLED,
};

struct ipconfig_profile{
	std::string device{};
	std::string profile{};
	struct{
		ipv4_mode mode=IPV4_DHCP;
		ipv4_cidr address{};
		ipv4 gateway{};
	}v4{};
	bool autodns=true;
	std::string dns1{};
	std::string dns2{};
	std::string gen_config();
	void configure();
};

class device_ipconfig{
	public:
		std::string name;
		std::string profile;
		std::function<void()>on_back=nullptr;
};

class ui_network_panel{
	public:
		ui_network_panel(ui_draw_network*net):net(net){}
		virtual void hide();
		virtual void show();
		virtual void draw(lv_obj_t*cont);
		virtual void draw_device(lv_obj_t*){}
		virtual void draw_setting(lv_obj_t*){}
		virtual bool have_back(){return false;}
		virtual void do_back(){}
		lv_obj_t*panel=nullptr;
		ui_draw_network*net;
};

template<typename T>
class ui_network_panel_device:public ui_network_panel{
	public:
		ui_network_panel_device(ui_draw_network*net):ui_network_panel(net){}
		inline void hide()override{
			ui_network_panel::hide();
			if(current)switch_to(nullptr);
		}
		virtual void switch_to(const std::shared_ptr<T>&dev){
			if(current==dev)return;
			current=dev;
			if(dev)show();
		}
		std::list<std::shared_ptr<T>>devices{};
		std::shared_ptr<T>current=nullptr;
};

class ui_network_panel_hello:public ui_network_panel{
	public:
		ui_network_panel_hello(ui_draw_network*net):ui_network_panel(net){}
		void draw(lv_obj_t*cont)override;
};

class ui_network_panel_wifi:public ui_network_panel_device<device_wifi>{
	public:
		ui_network_panel_wifi(ui_draw_network*net):ui_network_panel_device(net){}
		void thread_connect_wpa(std::shared_ptr<device_wifi>dev);
		void thread_scan();
		void update_status();
		void update_scan_result();
		void draw_ssid_items();
		void draw(lv_obj_t*cont)override;
		void draw_device(lv_obj_t*)override;
		void switch_to(const std::shared_ptr<device_wifi>&dev)override;
		void btn_rescan_clicked(lv_event_t*ev);
		void wlan_device_up_changed(lv_event_t*ev);
		void wlan_event_handle(const std::string&ev);
		const char*icon_from_result(std::shared_ptr<wlan_scan_result>result);
		bool updating=false;
		uint64_t event_id=UINT64_MAX;
		lv_obj_t*lbl_wlan_device_up=nullptr;
		lv_obj_t*wlan_device_up=nullptr;
		lv_obj_t*lbl_wlan_state=nullptr;
		lv_obj_t*lst_wifi=nullptr;
		lv_obj_t*btn_rescan=nullptr;
		lv_obj_t*lst_btn_connect_hidden=nullptr;
		struct ssid_item{
			bool connected=false;
			std::shared_ptr<wlan_scan_result>result=nullptr;
			lv_obj_t*lst_btn=nullptr;
			lv_obj_t*lbl_signal=nullptr;
			lv_obj_t*lbl_text=nullptr;
			lv_obj_t*lbl_connected=nullptr;
		};
		std::vector<ssid_item>ssid_items{64};
};

class ui_network_panel_wifi_connect:public ui_network_panel_device<device_wifi_ssid>{
	public:
		ui_network_panel_wifi_connect(ui_draw_network*net):ui_network_panel_device(net){}
		void draw(lv_obj_t*cont)override;
		void draw_buttons(lv_obj_t*cont);
		void switch_to(const std::shared_ptr<device_wifi_ssid>&dev)override;
		void load_address();
		void type_changed(lv_event_t*ev);
		void password_show_clicked(lv_event_t*ev);
		void btn_ipconfig_clicked(lv_event_t*ev);
		void btn_connect_clicked(lv_event_t*ev);
		void btn_disconnect_clicked(lv_event_t*ev);
		void worker_connect();
		void worker_disconnect();
		std::shared_ptr<wlan_network>create_network(bool lock=false);
		inline bool have_back()override{return true;}
		void do_back()override;
		void clear();
		void disable_all();
		void update();
		void show_spinner();
		bool updating=false;
		lv_timer_t*addr_timer=nullptr;
		lv_obj_t*ssid=nullptr;
		lv_obj_t*type=nullptr;
		lv_obj_t*password=nullptr;
		lv_obj_t*password_show=nullptr;
		lv_obj_t*btn_ip_config=nullptr;
		lv_obj_t*btn_disconnect=nullptr;
		lv_obj_t*btn_connect=nullptr;
		lv_obj_t*lbl_title=nullptr;
		lv_obj_t*lbl_ssid=nullptr;
		lv_obj_t*lbl_type=nullptr;
		lv_obj_t*lbl_password=nullptr;
		lv_obj_t*lbl_password_show=nullptr;
		lv_obj_t*lbl_address=nullptr;
		lv_obj_t*spinner=nullptr;
};

class ui_network_panel_eth:public ui_network_panel_device<device_eth>{
	public:
		ui_network_panel_eth(ui_draw_network*net):ui_network_panel_device(net){}
		~ui_network_panel_eth();
		void draw(lv_obj_t*cont)override;
		void draw_buttons(lv_obj_t*cont);
		void draw_device(lv_obj_t*)override;
		void switch_to(const std::shared_ptr<device_eth>&dev)override;
		void eth_device_up_changed(lv_event_t*ev);
		void btn_ipconfig_clicked(lv_event_t*ev);
		void btn_reconfigure_clicked(lv_event_t*ev);
		void update_status();
		void configure();
		lv_obj_t*lbl_eth_device_up=nullptr;
		lv_obj_t*eth_device_up=nullptr;
		lv_obj_t*lbl_eth_state=nullptr;
		lv_obj_t*lbl_address=nullptr;
		lv_obj_t*btn_ip_config=nullptr;
		lv_obj_t*btn_reconfigure=nullptr;
		lv_timer_t*timer=nullptr;
		uint64_t ipconfig_id=0;
		bool updating=false;
		bool configuring=false;
};

class ui_network_panel_proxy:public ui_network_panel{
	public:
		ui_network_panel_proxy(ui_draw_network*net):ui_network_panel(net){}
		void show()override;
		void reload();
		void clear();
		void draw(lv_obj_t*cont)override;
		void draw_buttons(lv_obj_t*cont);
		void draw_setting(lv_obj_t*cont)override;
		void btn_proxy_clicked(lv_event_t*ev);
		void btn_clear_clicked(lv_event_t*ev);
		void btn_apply_clicked(lv_event_t*ev);
		lv_obj_t*btn_panel_proxy=nullptr;
		lv_obj_t*all_proxy=nullptr;
		lv_obj_t*ftp_proxy=nullptr;
		lv_obj_t*http_proxy=nullptr;
		lv_obj_t*https_proxy=nullptr;
		lv_obj_t*no_proxy=nullptr;
		lv_obj_t*btn_clear=nullptr;
		lv_obj_t*btn_apply=nullptr;
};

class ui_network_panel_ipconfig:public ui_network_panel_device<device_ipconfig>{
	public:
		using profile_callback=std::function<void(
			const std::string&dev,
			const std::string&sub,
			ipconfig_profile&profile
		)>;
		ui_network_panel_ipconfig(ui_draw_network*net):ui_network_panel_device(net){}
		void reload();
		void reset();
		void update_status();
		void draw(lv_obj_t*cont)override;
		void draw_buttons(lv_obj_t*cont);
		void btn_reset_clicked(lv_event_t*ev);
		void btn_reload_clicked(lv_event_t*ev);
		void btn_apply_clicked(lv_event_t*ev);
		void switch_to(const std::shared_ptr<device_ipconfig>&dev)override;
		bool have_back()override;
		void do_back()override;
		uint64_t add_on_changes(const profile_callback&f);
		void remove_on_changes(uint64_t id);
		ipconfig_profile&get_profile();
		ipconfig_profile&find_profile(const std::string&dev,const std::string&sub="default");
		std::atomic<uint64_t>func_id=0;
		std::map<uint64_t,profile_callback>on_changes{};
		std::map<std::string,std::map<std::string,ipconfig_profile>>profiles{};
		lv_obj_t*lbl_title=nullptr;
		lv_obj_t*drop_ipv4_mode=nullptr;
		lv_obj_t*text_ipv4_address=nullptr;
		lv_obj_t*text_ipv4_netmask=nullptr;
		lv_obj_t*text_ipv4_gateway=nullptr;
		lv_obj_t*check_ipv4_autodns=nullptr;
		lv_obj_t*text_ipv4_dns1=nullptr;
		lv_obj_t*text_ipv4_dns2=nullptr;
		lv_obj_t*lbl_ipv4_mode=nullptr;
		lv_obj_t*lbl_ipv4_address=nullptr;
		lv_obj_t*lbl_ipv4_netmask=nullptr;
		lv_obj_t*lbl_ipv4_gateway=nullptr;
		lv_obj_t*lbl_ipv4_autodns=nullptr;
		lv_obj_t*lbl_ipv4_dns1=nullptr;
		lv_obj_t*lbl_ipv4_dns2=nullptr;
		lv_obj_t*btn_apply=nullptr;
		lv_obj_t*btn_reload=nullptr;
		lv_obj_t*btn_reset=nullptr;
};

class ui_draw_network:public ui_draw{
	public:
		void draw(lv_obj_t*cont)override;
		void draw_buttons(lv_obj_t*cont);
		void hide_all();
		void btn_back_cb(lv_event_t*ev);
		void btn_skip_cb(lv_event_t*ev);
		void btn_next_cb(lv_event_t*ev);
		lv_obj_t*panel=nullptr;
		lv_obj_t*btn_back=nullptr;
		lv_obj_t*btn_skip=nullptr;
		lv_obj_t*btn_next=nullptr;
		ui_network_panel*current=nullptr;
		ui_network_panel_hello hello{this};
		ui_network_panel_wifi wifi{this};
		ui_network_panel_wifi_connect wifi_connect{this};
		ui_network_panel_ipconfig ipconfig{this};
		ui_network_panel_eth eth{this};
		ui_network_panel_proxy proxy{this};
		const std::vector<ui_network_panel*>panels{
			&hello,&wifi,&wifi_connect,&ipconfig,&eth,&proxy
		};
};
#endif
