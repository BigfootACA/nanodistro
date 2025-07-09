#include"network.h"
#include"netif.h"
#include"configs.h"
#include"worker.h"
#include"fs-utils.h"
#include"std-utils.h"
#include"str-utils.h"
#include<dirent.h>
#include<net/if.h>
#include"log.h"

void ui_network_panel_eth::update_status(){
	if(!current||updating)return;
	updating=true;
	worker_add([this]{
		int flags=0;
		std::string addr{};
		try{
			network_helper net{};
			auto intf=net.get_intf(current->name);
			flags=intf.get_flags();
		}catch(std::exception&exc){
			log_exception(exc,"failed to update eth {} status",current->name.c_str());
			updating=false;
			return;
		}
		try{
			auto info=get_intf_address(current->name);
			for(auto&cidr:info.addr_v4)
				addr+=ssprintf(_("IPv4 Address: %s\n"),cidr.to_string().c_str());
			if(!info.gw_v4.empty())
				addr+=ssprintf(_("IPv4 Gateway: %s\n"),info.gw_v4.to_string().c_str());
			for(auto&cidr:info.addr_v6)
				addr+=ssprintf(_("IPv6 Address: %s\n"),cidr.to_string().c_str());
			if(!info.gw_v6.empty())
				addr+=ssprintf(_("IPv6 Gateway: %s\n"),info.gw_v6.to_string().c_str());
		}catch(std::exception&exc){
			log_exception(exc,"failed to get eth {} address",current->name.c_str());
		}
		if(fs_exists("/etc/resolv.conf"))try{
			resolv_conf conf;
			conf.load_file();
			for(auto ns:conf.nameservers)
				addr+=ssprintf(_("DNS: %s\n"),ns.c_str());
		}catch(std::exception&exc){
			log_exception(exc,"failed to load resolv.conf");
		}
		lv_thread_call_func([this,flags,addr]{
			bool is_up=have_bit(flags,IFF_UP);
			bool is_carrier=have_bit(flags,IFF_RUNNING);
			if(!lv_obj_has_state(eth_device_up,LV_STATE_DISABLED))
				lv_obj_set_checked(eth_device_up,is_up);
			const char*state;
			if(is_carrier)state=_("Connected");
			else if(is_up)state=_("No cable");
			else state=_("Disabled");
			lv_label_set_text_fmt(lbl_eth_state,_("Status: %s"),state);
			lv_label_set_text(lbl_address,addr.c_str());
			lv_obj_set_hidden(lbl_address,addr.empty());
			lv_obj_set_disabled(this->net->btn_next,!is_carrier||!is_up);
			updating=false;
		});
	});
}

void ui_network_panel_eth::configure(){
	if(!current||configuring)return;
	configuring=true;
	lv_obj_set_disabled(btn_ip_config,true);
	lv_obj_set_disabled(btn_reconfigure,true);
	worker_add([this]{
		try{
			auto profile=net->ipconfig.find_profile(current->name);
			log_info("configuring ethernet {}",current->name.c_str());
			profile.configure();
			log_info("configured ethernet {}",current->name.c_str());
		}catch(std::exception&exc){
			log_exception(exc,"failed to configure ethernet {}",current->name.c_str());
		}
		configuring=false;
		lv_thread_call_func([this]{
			lv_obj_set_disabled(btn_ip_config,false);
			lv_obj_set_disabled(btn_reconfigure,false);
			update_status();
		});
	});
}

void ui_network_panel_eth::eth_device_up_changed(lv_event_t*ev){
	if(!current)return;
	auto ifn=current->name;
	lv_obj_set_disabled(eth_device_up,true);
	bool up=lv_obj_has_state(eth_device_up,LV_STATE_CHECKED);
	worker_add([this,ifn,up]{
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
			log_exception(exc,"failed to set eth {} up",ifn);
		}
		if(run)try{
			xup=intf.is_up();
		}catch(std::exception&exc){
			log_exception(exc,"failed to set eth {} up",ifn);
		}
		lv_lock();
		if(xup)configure();
		lv_obj_set_checked(eth_device_up,xup);
		lv_obj_set_disabled(eth_device_up,false);
		lv_obj_set_disabled(this->net->btn_next,!xup);
		lv_unlock();
	});
}

void ui_network_panel_eth::btn_ipconfig_clicked(lv_event_t*){
	if(!current)return;
	auto ipconfig=std::make_shared<device_ipconfig>();
	ipconfig->name=current->name;
	ipconfig->on_back=[this,current=current]{
		if(current)switch_to(current);
	};
	net->ipconfig.switch_to(ipconfig);
}

void ui_network_panel_eth::btn_reconfigure_clicked(lv_event_t*){
	configure();
}

void ui_network_panel_eth::draw_buttons(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
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
	lv_obj_set_grid_cell(box,LV_GRID_ALIGN_STRETCH,0,2,LV_GRID_ALIGN_END,4,1);

	btn_ip_config=lv_button_create(box);
	auto lbl_btn_ip_config=lv_label_create(btn_ip_config);
	lv_obj_set_grid_cell(btn_ip_config,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_btn_ip_config,_("IP Config"));
	auto f1=std::bind(&ui_network_panel_eth::btn_ipconfig_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_ip_config,f1,LV_EVENT_CLICKED);

	btn_reconfigure=lv_button_create(box);
	auto lbl_btn_reconfigure=lv_label_create(btn_reconfigure);
	lv_obj_set_grid_cell(btn_reconfigure,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_btn_reconfigure,_("Reconfigure"));
	auto f2=std::bind(&ui_network_panel_eth::btn_reconfigure_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_reconfigure,f2,LV_EVENT_CLICKED);
}

void ui_network_panel_eth::draw(lv_obj_t*obj){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
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

	lbl_eth_device_up=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_eth_device_up,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,0,1);

	eth_device_up=lv_switch_create(panel);
	auto f1=std::bind(&ui_network_panel_eth::eth_device_up_changed,this,std::placeholders::_1);
	lv_obj_add_event_func(eth_device_up,f1,LV_EVENT_VALUE_CHANGED);
	lv_obj_set_grid_cell(eth_device_up,LV_GRID_ALIGN_END,1,1,LV_GRID_ALIGN_STRETCH,0,1);

	lbl_eth_state=lv_label_create(panel);
	lv_label_set_text_fmt(lbl_eth_state,_("Status: %s"),_("Unavailable"));
	lv_obj_set_grid_cell(lbl_eth_state,LV_GRID_ALIGN_START,0,2,LV_GRID_ALIGN_CENTER,1,1);

	lbl_address=lv_label_create(panel);
	lv_obj_set_hidden(lbl_address,true);
	lv_obj_set_grid_cell(lbl_address,LV_GRID_ALIGN_START,0,2,LV_GRID_ALIGN_CENTER,2,1);

	draw_buttons(panel);
}

void ui_network_panel_eth::draw_device(lv_obj_t*obj){
	if(auto v=config["nanodistro"]["network"]["ethernet"]["enabled"];!v||!v.as<bool>())return;
	DIR*d=opendir("/sys/class/net");
	if(!d)return;
	while(auto e=readdir(d)){
		if(e->d_name[0]=='.')continue;
		if(e->d_type!=DT_LNK)continue;
		std::string name=e->d_name;
		if(
			!name.starts_with("en")&&
			!name.starts_with("eth")&&
			!name.starts_with("usb")
		)continue;
		auto title=ssprintf(_("Ethernet: %s"),name.c_str());
		auto dev=std::make_shared<device_eth>();
		dev->name=name;
		dev->button=lv_list_add_button_ex(obj,"\xf3\xb0\x88\x80",title.c_str()); /* mdi-ethernet */
		auto f=std::bind(&ui_network_panel_eth::switch_to,this,dev);
		lv_obj_add_event_func(dev->button,f,LV_EVENT_CLICKED);
		devices.push_back(dev);
	}
	closedir(d);
}

void ui_network_panel_eth::switch_to(const std::shared_ptr<device_eth>&dev){
	if(timer){
		lv_timer_delete(timer);
		timer=nullptr;
	}
	if(ipconfig_id!=0){
		net->ipconfig.remove_on_changes(ipconfig_id);
		ipconfig_id=0;
	}
	ui_network_panel_device::switch_to(dev);
	if(current){
		lv_label_set_text_fmt(lbl_eth_device_up,_("Enable Ethernet %s"),current->name.c_str());
		lv_label_set_text_fmt(lbl_eth_state,_("Status: %s"),_("Unavailable"));
		update_status();
		timer=lv_timer_create([](auto tmr){
			auto c=(ui_network_panel_eth*)lv_timer_get_user_data(tmr);
			if(c)c->update_status();
		},1000,this);
		ipconfig_id=net->ipconfig.add_on_changes([this](auto dev,auto,auto profile){
			if(current&&current->name==dev)configure();
		});
	}
}

ui_network_panel_eth::~ui_network_panel_eth(){
	if(timer){
		lv_timer_delete(timer);
		timer=nullptr;
	}
	if(ipconfig_id!=0){
		net->ipconfig.remove_on_changes(ipconfig_id);
		ipconfig_id=0;
	}
}
