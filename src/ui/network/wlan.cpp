#include<format>
#include<cstring>
#include<algorithm>
#include"log.h"
#include"network.h"
#include"worker.h"
#include"wlan.h"
#include"netif.h"
#include"configs.h"
#include"str-utils.h"
#include"gui.h"

void ui_network_panel_wifi::btn_rescan_clicked(lv_event_t*){
	if(current&&current->client)worker_add([client=current->client]{
		try{
			client->start_scan();
		}catch(std::exception&exc){
			log_exception(exc,"failed to start wlan scan");
		}
	});
}

void ui_network_panel_wifi::wlan_device_up_changed(lv_event_t*){
	if(!current)return;
	auto ifn=current->name;
	auto client=current->client;
	lv_obj_set_disabled(wlan_device_up,true);
	bool up=lv_obj_has_state(wlan_device_up,LV_STATE_CHECKED);
	worker_add([ifn,up,wlan_device_up=wlan_device_up,lst_wifi=lst_wifi,client]{
		network_helper net(false);
		intf_helper intf;
		bool run=false,xup=up;
		try{
			net.init();
			intf=net.get_intf(ifn);
			run=true;
		}catch(std::exception&exc){
			log_exception(exc,"failed to set get interface {}",ifn);
		}
		if(run)try{
			intf.set_up(xup);
		}catch(std::exception&exc){
			log_exception(exc,"failed to set wlan {} up",ifn);
		}
		if(run)try{
			xup=intf.is_up();
		}catch(std::exception&exc){
			log_exception(exc,"failed to set wlan {} up",ifn);
		}
		if(run&&client&&xup)try{
			client->start_scan();
		}catch(std::exception&exc){
			log_exception(exc,"failed to start wlan scan");
		}
		lv_lock();
		lv_obj_set_checked(wlan_device_up,xup);
		lv_obj_set_disabled(wlan_device_up,false);
		lv_obj_set_disabled(lst_wifi,!xup);
		lv_unlock();
	});
}

void ui_network_panel_wifi::draw_ssid_items(){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};
	auto mdi=fonts_get("mdi-icon");
	for(auto&ssid:ssid_items){
		ssid.lst_btn=lv_obj_class_create_obj(&lv_list_button_class,lst_wifi);
		lv_obj_class_init_obj(ssid.lst_btn);
		lv_obj_set_grid_dsc_array(ssid.lst_btn,grid_cols,grid_rows);

		ssid.lbl_signal=lv_image_create(ssid.lst_btn);
		if(mdi)lv_obj_set_style_text_font(ssid.lbl_signal,mdi,0);
		lv_obj_set_grid_cell(ssid.lbl_signal,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,0,1);

		ssid.lbl_text=lv_label_create(ssid.lst_btn);
		lv_label_set_long_mode(ssid.lbl_text,LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
		lv_obj_set_flex_grow(ssid.lbl_text,1);
		lv_obj_set_grid_cell(ssid.lbl_text,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_CENTER,0,1);

		ssid.lbl_connected=lv_image_create(ssid.lst_btn);
		if(mdi)lv_obj_set_style_text_font(ssid.lbl_connected,mdi,0);
		lv_image_set_src(ssid.lbl_connected,"\xf3\xb0\x84\xac"); /* mdi-check */
		lv_obj_set_grid_cell(ssid.lbl_connected,LV_GRID_ALIGN_END,2,1,LV_GRID_ALIGN_CENTER,0,1);

		lv_obj_add_event_func(ssid.lst_btn,[this,&ssid](auto){
			if(!current||!ssid.result)return;
			auto dev=std::make_shared<device_wifi_ssid>();
			dev->wifi=current;
			dev->result=ssid.result;
			net->wifi_connect.switch_to(dev);
		},LV_EVENT_CLICKED);
	}

	lst_btn_connect_hidden=lv_list_add_button(lst_wifi,NULL,_("Connect to hidden network..."));
	lv_obj_add_event_func(lst_btn_connect_hidden,[this](auto){
		if(!current)return;
		auto dev=std::make_shared<device_wifi_ssid>();
		dev->wifi=current;
		net->wifi_connect.switch_to(dev);
	},LV_EVENT_CLICKED);
}

void ui_network_panel_wifi::draw(lv_obj_t*obj){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	};
	ui_network_panel::draw(obj);
	lv_obj_set_size(panel,lv_pct(100),lv_pct(100));
	lv_obj_set_grid_dsc_array(panel,grid_cols,grid_rows);

	lbl_wlan_device_up=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_wlan_device_up,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,0,1);

	wlan_device_up=lv_switch_create(panel);
	auto f1=std::bind(&ui_network_panel_wifi::wlan_device_up_changed,this,std::placeholders::_1);
	lv_obj_add_event_func(wlan_device_up,f1,LV_EVENT_VALUE_CHANGED);
	lv_obj_set_grid_cell(wlan_device_up,LV_GRID_ALIGN_END,1,1,LV_GRID_ALIGN_STRETCH,0,1);

	lbl_wlan_state=lv_label_create(panel);
	lv_label_set_text(lbl_wlan_state,_("Unavailable"));
	lv_obj_set_grid_cell(lbl_wlan_state,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,1,1);

	btn_rescan=lv_button_create(panel);
	auto lbl_btn_rescan=lv_label_create(btn_rescan);
	lv_obj_set_size(btn_rescan,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	auto mdi=fonts_get("mdi-icon");
	if(mdi)lv_obj_set_style_text_font(lbl_btn_rescan,mdi,0);
	lv_label_set_text(lbl_btn_rescan,"\xf3\xb0\x91\x90"); /* mdi-refresh */
	lv_obj_set_grid_cell(btn_rescan,LV_GRID_ALIGN_END,1,1,LV_GRID_ALIGN_CENTER,1,1);
	auto f2=std::bind(&ui_network_panel_wifi::btn_rescan_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_rescan,f2,LV_EVENT_CLICKED);

	lst_wifi=lv_list_create(panel);
	lv_obj_set_size(lst_wifi,lv_pct(100),lv_pct(100));
	lv_obj_set_grid_cell(lst_wifi,LV_GRID_ALIGN_STRETCH,0,2,LV_GRID_ALIGN_STRETCH,2,1);
	lv_obj_set_disabled(lst_wifi,false);
	draw_ssid_items();
}

void ui_network_panel_wifi::thread_connect_wpa(std::shared_ptr<device_wifi>dev){
	if(dev->client)return;
	try{
		dev->client=wlan_client::create(dev->name);
		dev->client->start_scan();
	}catch(std::exception&exc){
		log_exception(exc,"failed to create wlan client {}",dev->name);
		dev->client=nullptr;
		return;
	}
	if(dev==current){
		lv_lock();
		lv_label_set_text(lbl_wlan_state,_("Ready"));
		lv_unlock();
	}
}

void ui_network_panel_wifi::thread_scan(){
	if(!current||!current->client){
		updating=false;
		return;
	}
	try{
		current->results=current->client->scan_results();
		current->networks=current->client->list_networks();
		lv_thread_call_func(std::bind(&ui_network_panel_wifi::update_scan_result,this));
	}catch(std::exception&exc){
		log_exception(exc,"failed to scan wlan {}",current->name);
	}
	try{
		current->status=current->client->get_status();
		current->state=string_to_wpa_states(current->status["wpa_state"]);
		current->cur_bssid.clear();
		if(current->status.contains("bssid"))
			current->cur_bssid=current->status["bssid"];
		lv_thread_call_func(std::bind(&ui_network_panel_wifi::update_status,this));
	}catch(std::exception&exc){
		log_exception(exc,"failed to scan wlan {}",current->name);
	}
	updating=false;
}

static const char*wpa_readable_state(wpa_states state){
	switch(state){
		case WPA_DISCONNECTED:return _("Disconnected");
		case WPA_INTERFACE_DISABLED:return _("Disabled");
		case WPA_INACTIVE:return _("Disconnected");
		case WPA_SCANNING:return _("Scanning");
		case WPA_AUTHENTICATING:return _("Authenticating");
		case WPA_ASSOCIATING:return _("Associating");
		case WPA_ASSOCIATED:return _("Associated");
		case WPA_4WAY_HANDSHAKE:return _("Handshaking");
		case WPA_GROUP_HANDSHAKE:return _("Handshaking");
		case WPA_COMPLETED:return _("Connected");
		default:return _("Unknown");
	}
}

void ui_network_panel_wifi::update_status(){
	if(!current)return;
	if(!current->status.contains("wpa_state"))return;
	if(!lv_obj_has_state(wlan_device_up,LV_STATE_DISABLED))try{
		bool up=network_helper().is_intf_up(current->name);
		lv_obj_set_checked(wlan_device_up,up);
		lv_obj_set_disabled(lst_wifi,!up);
	}catch(std::exception&exc){
		log_exception(exc,"failed to update wlan {} status",current->name.c_str());
	}
	lv_label_set_text(lbl_wlan_state,wpa_readable_state(current->state));
	lv_obj_set_disabled(net->btn_next,current->state!=WPA_COMPLETED);
}

void ui_network_panel_wifi::wlan_event_handle(const std::string&ev){
	if(updating)return;
	static const std::vector matches{
		"CTRL-EVENT-SSID-TEMP-DISABLED",
		"CTRL-EVENT-SSID-ENABLED",
		"CTRL-EVENT-SSID-REENABLED",
		"CTRL-EVENT-SSID-CHANGED",
		"CTRL-EVENT-CONNECTED",
		"CTRL-EVENT-DISCONNECTED",
		"CTRL-EVENT-SCAN-RESULTS",
		"CTRL-EVENT-SCAN-STARTED",
		"CTRL-EVENT-SUBNET-STATUS-UPDATE",
		"CTRL-EVENT-NETWORK-NOT-FOUND",
	};
	if(std::none_of(matches.begin(),matches.end(),[ev](const auto&match){
		return ev.starts_with(match);
	}))return;
	updating=true;
	worker_add(std::bind(&ui_network_panel_wifi::thread_scan,this));
}

const char*ui_network_panel_wifi::icon_from_result(std::shared_ptr<wlan_scan_result>result){
	static const char*icons[]={
		"\xf3\xb0\xa4\xaf", /* mdi-wifi-strength-outline */
		"\xf3\xb0\xa4\x9f", /* mdi-wifi-strength-1 */
		"\xf3\xb0\xa4\xa2", /* mdi-wifi-strength-2 */
		"\xf3\xb0\xa4\xa5", /* mdi-wifi-strength-3 */
		"\xf3\xb0\xa4\xa8", /* mdi-wifi-strength-4 */
	},*icons_lock[]={
		"\xf3\xb0\xa4\xac", /* mdi-wifi-strength-lock-outline */
		"\xf3\xb0\xa4\xa1", /* mdi-wifi-strength-1-lock */
		"\xf3\xb0\xa4\xa4", /* mdi-wifi-strength-2-lock */
		"\xf3\xb0\xa4\xa7", /* mdi-wifi-strength-3-lock */
		"\xf3\xb0\xa4\xaa", /* mdi-wifi-strength-4-lock */
	};
	bool secure=result->is_flags_contains({"WPA","WEP"});
	auto level=std::min(std::max((result->rssi+80)/10,0),4);
	return (secure?icons_lock:icons)[level];
}

void ui_network_panel_wifi::update_scan_result(){
	if(!current)return;

	auto grp=lv_group_get_default();
	lv_coord_t sc=lv_obj_get_scroll_y(lst_wifi);
	std::string last_focus{};
	auto cur=lv_group_get_focused(grp);
	if(cur&&cur->class_p==&lv_list_button_class){
		for(auto&ssid:ssid_items)
			if(ssid.result&&ssid.lst_btn==cur)
				last_focus=ssid.result->ssid;
		if(!last_focus.empty())
			lv_group_focus_obj(btn_rescan);
	}

	std::vector<std::string>dedup{};
	std::vector<std::shared_ptr<wlan_scan_result>>xresults{
		current->results.begin(),
		current->results.end()
	};
	std::sort(xresults.begin(),xresults.end(),[](auto&lhs,auto&rhs){
		return lhs->rssi>rhs->rssi;
	});

	size_t ssid_idx=0;
	ssid_item*item_focus=nullptr;
	auto draw_result=[&](std::shared_ptr<wlan_scan_result>&result,bool connected=false){
		if(ssid_idx>=ssid_items.size())return;
		if(std::find(dedup.begin(),dedup.end(),result->ssid)!=dedup.end())return;
		dedup.push_back(result->ssid);
		ssid_item&ssid=ssid_items[ssid_idx++];
		ssid.result=result;
		ssid.connected=connected;
		if(result->ssid==last_focus)item_focus=&ssid;
		lv_obj_set_hidden(ssid.lst_btn,false);
		lv_image_set_src(ssid.lbl_signal,icon_from_result(result));
		lv_label_set_text(ssid.lbl_text,result->ssid.c_str());
		lv_obj_set_hidden(ssid.lbl_connected,!connected);
	};

	if(!current->cur_bssid.empty())
		for(auto&result:current->results)
			if(result->bssid==current->cur_bssid)
				draw_result(result,true);
	for(auto&result:xresults)
		draw_result(result);

	for(size_t i=ssid_idx;i<ssid_items.size();i++){
		auto&ssid=ssid_items[i];
		ssid.result=nullptr;
		ssid.connected=false;
		lv_obj_set_hidden(ssid.lst_btn,true);
	}

	if(last_focus.empty())
		lv_obj_scroll_to_y(lst_wifi,sc,LV_ANIM_OFF);
	else if(item_focus){
		lv_group_focus_obj(item_focus->lst_btn);
		lv_obj_add_state(item_focus->lst_btn,LV_STATE_FOCUS_KEY);
		lv_obj_scroll_to_view(item_focus->lst_btn,LV_ANIM_OFF);
	}
}

void ui_network_panel_wifi::draw_device(lv_obj_t*obj){
	if(auto v=config["nanodistro"]["network"]["wlan"]["enabled"];!v||!v.as<bool>())return;
	for(auto&name:wlan_client::list_wlan_devices()){
		auto title=ssprintf(_("WLAN: %s"),name.c_str());
		auto dev=std::make_shared<device_wifi>();
		dev->name=name;
		dev->button=lv_list_add_button_ex(obj,"\xf3\xb0\x96\xa9",title.c_str()); /* mdi-wifi */
		auto f=std::bind(&ui_network_panel_wifi::switch_to,this,dev);
		lv_obj_add_event_func(dev->button,f,LV_EVENT_CLICKED);
		devices.push_back(dev);
	}
	for(auto&dev:devices)
		worker_add(std::bind(&ui_network_panel_wifi::thread_connect_wpa,this,dev));
}

void ui_network_panel_wifi::switch_to(const std::shared_ptr<device_wifi>&dev){
	if(!dev&&current&&current->client&&event_id!=UINT64_MAX){
		current->client->unlisten_event(event_id);
		event_id=UINT64_MAX;
	}
	ui_network_panel_device::switch_to(dev);
	if(current){
		lv_group_focus_obj(btn_rescan);
		lv_label_set_text_fmt(lbl_wlan_device_up,_("Enable WLAN %s"),current->name.c_str());
		lv_label_set_text(lbl_wlan_state,current->client?_("Ready"):_("Initializing"));
		update_scan_result();
		update_status();
		if(event_id==UINT64_MAX&&current->client){
			auto f=std::bind(&ui_network_panel_wifi::wlan_event_handle,this,std::placeholders::_2);
			event_id=current->client->listen_event(f);
		}
		updating=true;
		worker_add(std::bind(&ui_network_panel_wifi::thread_scan,this));
	}
}
