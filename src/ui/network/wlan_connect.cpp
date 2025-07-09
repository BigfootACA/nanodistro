#include"network.h"
#include<dirent.h>
#include"str-utils.h"
#include"fs-utils.h"
#include"worker.h"
#include"netif.h"
#include"error.h"
#include"wlan.h"
#include"log.h"
#include"gui.h"

enum security_type{
	SEC_NONE,
	SEC_WPA_PSK,
};

void ui_network_panel_wifi_connect::type_changed(lv_event_t*){
	auto hide=lv_dropdown_get_selected(type)==SEC_NONE;
	lv_obj_set_hidden(lbl_password,hide);
	lv_obj_set_hidden(password,hide);
	lv_obj_set_hidden(password_show,hide);
}

void ui_network_panel_wifi_connect::password_show_clicked(lv_event_t*){
	auto show=lv_obj_has_state(password_show,LV_STATE_CHECKED);
	lv_textarea_set_password_mode(password,!show);
	lv_label_set_text(lbl_password_show,show?"\xf3\xb0\x88\x88":"\xf3\xb0\x88\x89"); /* mdi-eye mdi-eye-off */
}

void ui_network_panel_wifi_connect::btn_ipconfig_clicked(lv_event_t*){
	if(!current||!current->wifi)return;
	auto ipconfig=std::make_shared<device_ipconfig>();
	ipconfig->name=current->wifi->name;
	ipconfig->on_back=[this,current=current]{
		if(current)switch_to(current);
	};
	net->ipconfig.switch_to(ipconfig);
}

void ui_network_panel_wifi_connect::btn_connect_clicked(lv_event_t*){
	if(!current||!current->wifi||!current->wifi->client)return;
	log_info("button request connect to network");
	std::string ssid_str=lv_textarea_get_text(ssid);
	std::string password_str=lv_textarea_get_text(password);
	security_type sec_type=(security_type)lv_dropdown_get_selected(type);
	if(ssid_str.empty()){
		msgbox_show(_("SSID is empty"));
		return;
	}
	switch(sec_type){
		case SEC_WPA_PSK:
			if(password_str.length()>=8&&password_str.length()<=63)break;
			msgbox_show(_("WPA key must be 8 to 63 characters"));
			return;
		break;
		case SEC_NONE:break;
		default:msgbox_show(_("Unknown security type"));return;
	}
	show_spinner();
	worker_add(std::bind(&ui_network_panel_wifi_connect::worker_connect,this));
}

std::shared_ptr<wlan_network>ui_network_panel_wifi_connect::create_network(bool lock){
	if(lock)lv_lock();
	std::string ssid_str=lv_textarea_get_text(ssid);
	std::string password_str=lv_textarea_get_text(password);
	security_type sec_type=(security_type)lv_dropdown_get_selected(type);
	if(lock)lv_unlock();
	auto n=current->wifi->client->add_network();
	n->set_str_value("ssid",ssid_str);
	log_info("adding network {} with id {}",ssid_str,n->item.id);
	switch(sec_type){
		case SEC_WPA_PSK:
			log_info("network {} using wpa psk",ssid_str);
			n->set_value("key_mgmt","WPA-PSK");
			n->set_str_value("psk",password_str);
		break;
		case SEC_NONE:
			log_info("network {} using open",ssid_str);
			n->set_value("key_mgmt","NONE");
		break;
		default:throw InvalidArgument("unknown security type");
	}
	return n;
}

static std::string disable_reason_to_string(const std::string&reason){
	if(reason=="NO_PSK_AVAILABLE")return _("No password available");
	if(reason=="CONN_FAILED")return _("Connection failed");
	if(reason=="WRONG_KEY")return _("Wrong password");
	if(reason=="AUTH_FAILED")return _("Authentication failed");
	return reason;
}

void ui_network_panel_wifi_connect::worker_connect(){
	lv_lock();
	std::string ssid_str=lv_textarea_get_text(ssid);
	lv_unlock();
	if(current->network){
		log_info("removing old network {} id {}",ssid_str,current->network->item.id);
		try{current->network->disable();}catch(...){}
		try{current->network->remove();}catch(...){}
		current->network=nullptr;
	}
	uint32_t id=0;
	try{
		auto n=create_network(true);
		log_info("selecting network {} id {}",ssid_str,n->item.id);
		current->network=n;
		id=n->item.id;
		n->select();
	}catch(std::exception&exc){
		log_exception(exc,"failed to connect network to {}",ssid_str);
		std::string reason=exc.what();
		lv_thread_call_func([this,ssid_str,reason]{	
			msgbox_show(ssprintf(
				_("Failed to connect to network %s: %s"),
				ssid_str.c_str(),reason.c_str()
			));
			update();
		});
		return;
	}
	auto do_configure=[this,ssid_str,id]{
		log_info("connected to network {} id {}",ssid_str,id);
		auto profile=net->ipconfig.find_profile(current->wifi->name,ssid_str);
		try{
			profile.configure();
		}catch(std::exception&exc){
			log_exception(exc,"failed to configure network {}",ssid_str);
			lv_thread_call_func([this,ssid_str,exc]{	
				msgbox_show(ssprintf(
					_("Failed to configure network %s: %s"),
					ssid_str.c_str(),exc.what()
				));
				update();
			});
			return;
		}
		auto done=std::bind(&ui_network_panel_wifi_connect::do_back,this);
		lv_thread_call_func(done);
	};
	auto on_fail=[this,ssid_str,id](std::string msg){
		log_info("failed to connect to network {} id {}: {}",ssid_str,id,msg);
		lv_thread_call_func([this,ssid_str,msg]{
			msgbox_show(ssprintf(
				_("Failed to connect to network %s: %s"),
				ssid_str.c_str(),msg.c_str()
			));
			update();
		});
	};
	auto start_time=time(nullptr);
	current->wifi->client->listen_event([
		ssid_str,start_time,do_configure,on_fail,current=current
	](auto id,auto ev){
		if(ev.starts_with("CTRL-EVENT-CONNECTED")){
			do_configure();
		}else if(ev.starts_with("CTRL-EVENT-SSID-TEMP-DISABLED ")){
			auto params=parse_environ(ev.substr(30),' ','=',false);
			std::string reason=_("Unknown");
			auto rssid=std::format("\"{}\"",ssid_str);
			if(params.contains("ssid")&&params["ssid"]!=rssid)
				return;
			if(params.contains("reason"))
				reason=disable_reason_to_string(params["reason"]);
			on_fail(reason);
			if(current->network)current->network->disable();
		}else if(ev.starts_with("CTRL-EVENT-NETWORK-NOT-FOUND")){
			auto now=time(nullptr);
			if(now-start_time<10)return;
			on_fail(_("Network not found"));
		}else return;
		current->wifi->client->unlisten_event(id);
	});
}

void ui_network_panel_wifi_connect::btn_disconnect_clicked(lv_event_t*){
	if(!current||!current->network)return;
	log_info("button request disconnect from network");
	show_spinner();
	worker_add(std::bind(&ui_network_panel_wifi_connect::worker_disconnect,this));
}

void ui_network_panel_wifi_connect::worker_disconnect(){
	auto on_done=[this]{
		update();
		do_back();
	};
	auto on_fail=[this]{
		update();
		msgbox_show(_("Failed to disconnect from network"));
	};
	try{
		log_info("disconnecting network {}",current->network->item.id);
		current->network->disable();
	}catch(std::exception&exc){
		log_exception(exc,"failed to disable network");
		lv_thread_call_func(on_fail);
		return;
	}
	lv_thread_call_func(on_done);
}

void ui_network_panel_wifi_connect::draw_buttons(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_FR(1),
		LV_GRID_FR(1),
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};

	auto box=lv_obj_create(cont);
	lv_obj_set_size(box,lv_pct(100),LV_SIZE_CONTENT);
	lv_obj_set_style_radius(box,0,0);
	lv_obj_set_style_pad_all(box,lv_dpx(6),0);
	lv_obj_set_style_bg_opa(box,0,0);
	lv_obj_set_style_border_width(box,0,0);
	lv_obj_set_grid_dsc_array(box,grid_cols,grid_rows);
	lv_obj_set_grid_cell(box,LV_GRID_ALIGN_STRETCH,0,3,LV_GRID_ALIGN_END,6,1);

	btn_ip_config=lv_button_create(box);
	auto lbl_btn_ip_config=lv_label_create(btn_ip_config);
	lv_obj_set_grid_cell(btn_ip_config,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_btn_ip_config,_("IP Config"));
	auto f1=std::bind(&ui_network_panel_wifi_connect::btn_ipconfig_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_ip_config,f1,LV_EVENT_CLICKED);

	btn_disconnect=lv_button_create(box);
	auto lbl_btn_disconnect=lv_label_create(btn_disconnect);
	lv_obj_set_grid_cell(btn_disconnect,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_obj_set_disabled(btn_disconnect,true);
	lv_label_set_text(lbl_btn_disconnect,_("Disconnect"));
	auto f2=std::bind(&ui_network_panel_wifi_connect::btn_disconnect_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_disconnect,f2,LV_EVENT_CLICKED);

	btn_connect=lv_button_create(box);
	auto lbl_btn_connect=lv_label_create(btn_connect);
	lv_obj_set_grid_cell(btn_connect,LV_GRID_ALIGN_CENTER,2,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_obj_set_disabled(btn_connect,true);
	lv_label_set_text(lbl_btn_connect,_("Connect"));
	auto f3=std::bind(&ui_network_panel_wifi_connect::btn_connect_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_connect,f3,LV_EVENT_CLICKED);
}

void ui_network_panel_wifi_connect::draw(lv_obj_t*obj){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};
	ui_network_panel::draw(obj);
	lv_obj_set_size(panel,lv_pct(100),lv_pct(100));
	lv_obj_set_grid_dsc_array(panel,grid_cols,grid_rows);

	lbl_title=lv_label_create(panel);
	lv_obj_set_style_margin_all(lbl_title,28,0);
	lv_obj_set_grid_cell(lbl_title,LV_GRID_ALIGN_CENTER,0,3,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_title,_("Connect to WiFi"));

	lbl_ssid=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_ssid,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_label_set_text(lbl_ssid,_("Network SSID"));

	ssid=lv_textarea_create(panel);
	lv_textarea_set_one_line(ssid,true);
	lv_obj_set_grid_cell(ssid,LV_GRID_ALIGN_STRETCH,1,2,LV_GRID_ALIGN_STRETCH,1,1);
	inputbox_bind_textarea(_("Network SSID"),ssid);

	lbl_type=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_type,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,2,1);
	lv_label_set_text(lbl_type,_("Security type"));

	type=lv_dropdown_create(panel);
	lv_dropdown_clear_options(type);
	lv_dropdown_add_option(type,_("Open network"),SEC_NONE);
	lv_dropdown_add_option(type,_("WPA/WPA2/WPA3 Personal PSK"),SEC_WPA_PSK);
	lv_obj_set_grid_cell(type,LV_GRID_ALIGN_STRETCH,1,2,LV_GRID_ALIGN_STRETCH,2,1);
	auto mdis=fonts_get("mdi-normal");
	if(mdis)lv_obj_set_style_text_font(type,mdis,LV_PART_INDICATOR);
	auto f1=std::bind(&ui_network_panel_wifi_connect::type_changed,this,std::placeholders::_1);
	lv_obj_add_event_func(type,f1,LV_EVENT_VALUE_CHANGED);

	lbl_password=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_password,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,3,1);
	lv_label_set_text(lbl_password,_("Password"));

	password=lv_textarea_create(panel);
	lv_textarea_set_one_line(password,true);
	lv_obj_set_grid_cell(password,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,3,1);
	lv_textarea_set_password_mode(password,true);
	inputbox_bind_textarea(_("Password"),password);

	password_show=lv_button_create(panel);
	lbl_password_show=lv_label_create(password_show);
	lv_obj_set_grid_cell(password_show,LV_GRID_ALIGN_END,2,1,LV_GRID_ALIGN_STRETCH,3,1);
	auto c=lv_obj_get_style_bg_color(password_show,0);
	lv_obj_set_style_bg_color(password_show,c,LV_STATE_CHECKED);
	lv_obj_add_flag(password_show,LV_OBJ_FLAG_CHECKABLE);
	lv_label_set_text(lbl_password_show,"\xf3\xb0\x88\x89"); /* mdi-eye-off */
	auto mdil=fonts_get("mdi-icon");
	if(mdil)lv_obj_set_style_text_font(lbl_password_show,mdil,0);
	auto f2=std::bind(&ui_network_panel_wifi_connect::password_show_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(password_show,f2,LV_EVENT_VALUE_CHANGED);

	lbl_address=lv_label_create(panel);
	lv_obj_set_hidden(lbl_address,true);
	lv_obj_set_grid_cell(lbl_address,LV_GRID_ALIGN_START,0,3,LV_GRID_ALIGN_CENTER,4,1);

	spinner=lv_spinner_create(panel);
	lv_obj_set_hidden(spinner,true);
	lv_obj_center(spinner);

	draw_buttons(panel);
	clear();
}

void ui_network_panel_wifi_connect::disable_all(){
	lv_obj_set_disabled(ssid,true);
	lv_obj_set_disabled(type,true);
	lv_obj_set_disabled(password,true);
	lv_obj_set_disabled(password_show,true);
	lv_obj_set_disabled(btn_ip_config,true);
	lv_obj_set_disabled(btn_connect,true);
	lv_obj_set_disabled(btn_disconnect,true);
}

void ui_network_panel_wifi_connect::clear(){
	disable_all();
	lv_textarea_set_text(ssid,"");
	lv_textarea_set_text(password,"test");
	lv_dropdown_set_selected(type,SEC_NONE,LV_ANIM_OFF);
	lv_obj_set_hidden(lbl_password,true);
	lv_obj_set_hidden(password,true);
	lv_obj_set_hidden(password_show,true);
	lv_obj_set_hidden(spinner,true);
	lv_obj_set_disabled(password_show,false);
	lv_obj_set_disabled(btn_ip_config,false);
}

void ui_network_panel_wifi_connect::show_spinner(){
	disable_all();
	lv_obj_set_hidden(spinner,false);
}

void ui_network_panel_wifi_connect::update(){
	clear();
	if(!current)return;
	lv_obj_set_disabled(ssid,!!current->result);
	lv_obj_set_disabled(type,!!current->result);
	security_type sec_type=SEC_NONE;
	bool connected_current=current->status==device_wifi_ssid::CONNECTED_CURRENT;
	lv_obj_set_disabled(btn_connect,connected_current);
	lv_obj_set_disabled(btn_disconnect,!connected_current);
	if(current->result){
		if(current->result->is_flag_contains("WPA"))
			sec_type=SEC_WPA_PSK;
		lv_dropdown_set_selected(type,sec_type,LV_ANIM_OFF);
		lv_textarea_set_text(ssid,current->result->ssid.c_str());
		lv_obj_set_disabled(password,sec_type==SEC_NONE||connected_current);
		type_changed(nullptr);
		if(sec_type!=SEC_NONE)
			lv_group_focus_obj(type);
		else if(!connected_current)
			lv_group_focus_obj(btn_connect);
		else lv_group_focus_obj(net->btn_back);
	}else{
		lv_obj_set_disabled(password,connected_current);
		lv_group_focus_obj(type);
	}
}

void ui_network_panel_wifi_connect::load_address(){
	if(updating)return;
	if(!current||!current->wifi)return;
	updating=true;
	auto info=get_intf_address(current->wifi->name);
	std::string addr{};
	for(auto&cidr:info.addr_v4)
		addr+=ssprintf(_("IPv4 Address: %s\n"),cidr.to_string().c_str());
	if(!info.gw_v4.empty())
		addr+=ssprintf(_("IPv4 Gateway: %s\n"),info.gw_v4.to_string().c_str());
	for(auto&cidr:info.addr_v6)
		addr+=ssprintf(_("IPv6 Address: %s\n"),cidr.to_string().c_str());
	if(!info.gw_v6.empty())
		addr+=ssprintf(_("IPv6 Gateway: %s\n"),info.gw_v6.to_string().c_str());
	if(fs_exists("/etc/resolv.conf"))try{
		resolv_conf conf;
		conf.load_file();
		for(auto ns:conf.nameservers)
			addr+=ssprintf(_("DNS: %s\n"),ns.c_str());
	}catch(std::exception&exc){
		log_exception(exc,"failed to load resolv.conf");
	}
	lv_lock();
	lv_label_set_text(lbl_address,addr.c_str());
	lv_obj_set_hidden(lbl_address,false);
	lv_unlock();
	updating=false;
}

void ui_network_panel_wifi_connect::switch_to(const std::shared_ptr<device_wifi_ssid>&dev){
	if(!dev&&addr_timer){
		lv_timer_delete(addr_timer);
		addr_timer=nullptr;
		lv_obj_set_hidden(lbl_address,true);
	}
	ui_network_panel_device::switch_to(dev);
	clear();
	if(!current||!current->wifi)return;
	auto connected=!current->wifi->cur_bssid.empty();
	if(!current->result)log_info("connect to hidden ssid");
	else log_info("connect to ssid {}",current->result->ssid);
	log_info("wifi connected: {}",connected);
	current->status=connected?device_wifi_ssid::CONNECTED_OTHER:device_wifi_ssid::DISCONNECTED;
	if(!current->network&&current->result)for(auto&r:current->wifi->networks){
		if(r->item.ssid!=current->result->ssid)continue;
		current->network=r;
		if(r->item.is_flag("CURRENT")){
			log_info("found current network {} id {}",r->item.ssid,r->item.id);
			current->status=device_wifi_ssid::CONNECTED_CURRENT;
			break;
		}
	}
	if(current->status==device_wifi_ssid::CONNECTED_CURRENT&&!addr_timer){
		addr_timer=lv_timer_create([](lv_timer_t*timer){
			auto obj=(ui_network_panel_wifi_connect*)lv_timer_get_user_data(timer);
			if(obj)worker_add(std::bind(&ui_network_panel_wifi_connect::load_address,obj));
		},1000,this);
		worker_add(std::bind(&ui_network_panel_wifi_connect::load_address,this));
	}
	update();
}

void ui_network_panel_wifi_connect::do_back(){
	if(!net||!current||!current->wifi)return;
	net->wifi.switch_to(current->wifi);
}
