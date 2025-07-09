#include"log.h"
#include"error.h"
#include"network.h"
#include"configs.h"
#include"process.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"path-utils.h"

void ui_network_panel_ipconfig::reload(){
	if(!current)return;
	auto profile=get_profile();
	lv_dropdown_set_selected(drop_ipv4_mode,profile.v4.mode,LV_ANIM_OFF);
	lv_textarea_set_text(text_ipv4_address,profile.v4.address.addr.empty()?"":
		profile.v4.address.addr.to_string().c_str());
	lv_textarea_set_text(text_ipv4_netmask,profile.v4.address.mask.empty()?"":
		profile.v4.address.mask.to_string().c_str());
	lv_textarea_set_text(text_ipv4_gateway,profile.v4.gateway.empty()?"":
		profile.v4.gateway.to_string().c_str());
	lv_textarea_set_text(text_ipv4_dns1,profile.dns1.c_str());
	lv_textarea_set_text(text_ipv4_dns2,profile.dns2.c_str());
	lv_obj_set_checked(check_ipv4_autodns,profile.autodns);
	update_status();
}

void ui_network_panel_ipconfig::reset(){
	if(!current)return;
	auto profile=get_profile();
	profile.v4.mode=IPV4_DHCP;
	profile.v4.address.clear();
	profile.v4.gateway.clear();
	profile.autodns=true;
	profile.dns1.clear();
	profile.dns2.clear();
	for(auto&f:on_changes)f.second(current->name,current->profile,profile);
	reload();
}

void ui_network_panel_ipconfig::update_status(){
	if(!current)return;
	bool is_static=lv_dropdown_get_selected(drop_ipv4_mode)==IPV4_STATIC;
	bool is_dhcp=lv_dropdown_get_selected(drop_ipv4_mode)==IPV4_DHCP;
	bool is_disabled=lv_dropdown_get_selected(drop_ipv4_mode)==IPV4_DISABLED;
	bool is_autodns=lv_obj_has_state(check_ipv4_autodns,LV_STATE_CHECKED);
	if(is_static){
		lv_obj_remove_state(check_ipv4_autodns,LV_STATE_CHECKED);
		is_autodns=false;
	}
	lv_obj_set_hidden(lbl_ipv4_address,!is_static);
	lv_obj_set_hidden(text_ipv4_address,!is_static);
	lv_obj_set_hidden(lbl_ipv4_netmask,!is_static);
	lv_obj_set_hidden(text_ipv4_netmask,!is_static);
	lv_obj_set_hidden(lbl_ipv4_gateway,!is_static);
	lv_obj_set_hidden(text_ipv4_gateway,!is_static);
	lv_obj_set_hidden(lbl_ipv4_autodns,is_disabled);
	lv_obj_set_hidden(check_ipv4_autodns,is_disabled);
	lv_obj_set_disabled(check_ipv4_autodns,!is_dhcp);
	lv_obj_set_hidden(lbl_ipv4_dns1,is_disabled||is_autodns);
	lv_obj_set_hidden(text_ipv4_dns1,is_disabled||is_autodns);
	lv_obj_set_hidden(lbl_ipv4_dns2,is_disabled||is_autodns);
	lv_obj_set_hidden(text_ipv4_dns2,is_disabled||is_autodns);
}

uint64_t ui_network_panel_ipconfig::add_on_changes(const profile_callback&f){
	auto id=++func_id;
	on_changes[id]=f;
	return id;
}

void ui_network_panel_ipconfig::remove_on_changes(uint64_t id){
	auto it=on_changes.find(id);
	if(it!=on_changes.end())
		on_changes.erase(it);
}

void ui_network_panel_ipconfig::btn_reset_clicked(lv_event_t*){
	reset();
}

void ui_network_panel_ipconfig::btn_reload_clicked(lv_event_t*){
	reload();
}

void ui_network_panel_ipconfig::btn_apply_clicked(lv_event_t*){
	ipconfig_profile profile=get_profile();
	try{
		auto mode=(ipv4_mode)lv_dropdown_get_selected(drop_ipv4_mode);
		switch(mode){
			case IPV4_DHCP:
			case IPV4_STATIC:
			case IPV4_DISABLED:break;
			default:throw InvalidArgument("invalid ipv4 mode");
		}
		profile.v4.mode=mode;
		profile.v4.address.clear();
		profile.v4.gateway.clear();
		profile.autodns=true;
		profile.dns1.clear();
		profile.dns2.clear();
		if(mode==IPV4_STATIC){
			profile.v4.address=ipv4_cidr(lv_textarea_get_text(text_ipv4_address));
			profile.v4.gateway=ipv4(lv_textarea_get_text(text_ipv4_gateway));
		}
		if(mode!=IPV4_DISABLED){
			profile.autodns=!lv_obj_has_state(check_ipv4_autodns,LV_STATE_CHECKED);
			if(!profile.autodns){
				profile.dns1=lv_textarea_get_text(text_ipv4_dns1);
				profile.dns2=lv_textarea_get_text(text_ipv4_dns2);
			}
		}
	}catch(std::exception&exc){
		log_exception(exc,"failed to parse ipconfig");
		msgbox_show(ssprintf(_("Failed to parse ipconfig: %s"),exc.what()));
		return;
	}
	get_profile()=profile;
	for(auto&f:on_changes)f.second(current->name,current->profile,profile);
}

void ui_network_panel_ipconfig::draw_buttons(lv_obj_t*cont){
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
	lv_obj_set_grid_cell(box,LV_GRID_ALIGN_STRETCH,0,3,LV_GRID_ALIGN_END,9,1);

	btn_apply=lv_button_create(box);
	auto lbl_btn_apply=lv_label_create(btn_apply);
	lv_obj_set_grid_cell(btn_apply,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_btn_apply,_("Apply"));
	auto f1=std::bind(&ui_network_panel_ipconfig::btn_apply_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_apply,f1,LV_EVENT_CLICKED);

	btn_reload=lv_button_create(box);
	auto lbl_btn_reload=lv_label_create(btn_reload);
	lv_obj_set_grid_cell(btn_reload,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_btn_reload,_("Reload"));
	auto f2=std::bind(&ui_network_panel_ipconfig::btn_reload_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_reload,f2,LV_EVENT_CLICKED);

	btn_reset=lv_button_create(box);
	auto lbl_btn_reset=lv_label_create(btn_reset);
	lv_obj_set_grid_cell(btn_reset,LV_GRID_ALIGN_CENTER,2,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_btn_reset,_("Reset"));
	auto f3=std::bind(&ui_network_panel_ipconfig::btn_reset_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_reset,f3,LV_EVENT_CLICKED);
}

void ui_network_panel_ipconfig::draw(lv_obj_t*obj){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
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
	lv_obj_set_grid_dsc_array(panel,grid_cols,grid_rows);

	lbl_title=lv_label_create(panel);
	lv_obj_set_style_margin_all(lbl_title,28,0);
	lv_obj_set_style_pad_all(lbl_title,lv_dpx(8),0);
	lv_obj_set_grid_cell(lbl_title,LV_GRID_ALIGN_CENTER,0,4,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_title,_("Configure IP address"));

	lbl_ipv4_mode=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_ipv4_mode,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_label_set_text(lbl_ipv4_mode,_("IPv4 Mode"));

	drop_ipv4_mode=lv_dropdown_create(panel);
	lv_dropdown_clear_options(drop_ipv4_mode);
	lv_dropdown_add_option(drop_ipv4_mode,_("DHCP"),IPV4_DHCP);
	lv_dropdown_add_option(drop_ipv4_mode,_("Static"),IPV4_STATIC);
	lv_dropdown_add_option(drop_ipv4_mode,_("Disabled"),IPV4_DISABLED);
	lv_obj_set_grid_cell(drop_ipv4_mode,LV_GRID_ALIGN_STRETCH,1,3,LV_GRID_ALIGN_STRETCH,1,1);
	auto f1=std::bind(&ui_network_panel_ipconfig::update_status,this);
	lv_obj_add_event_func(drop_ipv4_mode,f1,LV_EVENT_VALUE_CHANGED);

	lbl_ipv4_address=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_ipv4_address,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,2,1);
	lv_label_set_text(lbl_ipv4_address,_("IPv4 Address"));

	text_ipv4_address=lv_textarea_create(panel);
	lv_textarea_set_one_line(text_ipv4_address,true);
	lv_obj_set_grid_cell(text_ipv4_address,LV_GRID_ALIGN_STRETCH,1,3,LV_GRID_ALIGN_STRETCH,2,1);
	inputbox_bind_textarea(_("IPv4 Address"),text_ipv4_address);

	lbl_ipv4_netmask=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_ipv4_netmask,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,3,1);
	lv_label_set_text(lbl_ipv4_netmask,_("IPv4 Netmask"));

	text_ipv4_netmask=lv_textarea_create(panel);
	lv_textarea_set_one_line(text_ipv4_netmask,true);
	lv_obj_set_grid_cell(text_ipv4_netmask,LV_GRID_ALIGN_STRETCH,1,3,LV_GRID_ALIGN_STRETCH,3,1);
	inputbox_bind_textarea(_("IPv4 Netmask"),text_ipv4_netmask);

	lbl_ipv4_gateway=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_ipv4_gateway,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,4,1);
	lv_label_set_text(lbl_ipv4_gateway,_("IPv4 Gateway"));

	text_ipv4_gateway=lv_textarea_create(panel);
	lv_textarea_set_one_line(text_ipv4_gateway,true);
	lv_obj_set_grid_cell(text_ipv4_gateway,LV_GRID_ALIGN_STRETCH,1,3,LV_GRID_ALIGN_STRETCH,4,1);
	inputbox_bind_textarea(_("IPv4 Gateway"),text_ipv4_gateway);

	lbl_ipv4_autodns=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_ipv4_autodns,LV_GRID_ALIGN_START,0,3,LV_GRID_ALIGN_CENTER,5,1);
	lv_label_set_text(lbl_ipv4_autodns,_("IPv4 Auto configure DNS"));

	check_ipv4_autodns=lv_switch_create(panel);
	lv_obj_set_grid_cell(check_ipv4_autodns,LV_GRID_ALIGN_CENTER,3,1,LV_GRID_ALIGN_STRETCH,5,1);
	auto f2=std::bind(&ui_network_panel_ipconfig::update_status,this);
	lv_obj_add_event_func(check_ipv4_autodns,f2,LV_EVENT_VALUE_CHANGED);

	lbl_ipv4_dns1=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_ipv4_dns1,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,6,1);
	lv_label_set_text(lbl_ipv4_dns1,_("IPv4 DNS1"));

	text_ipv4_dns1=lv_textarea_create(panel);
	lv_textarea_set_one_line(text_ipv4_dns1,true);
	lv_obj_set_grid_cell(text_ipv4_dns1,LV_GRID_ALIGN_STRETCH,1,3,LV_GRID_ALIGN_STRETCH,6,1);
	inputbox_bind_textarea(_("IPv4 DNS1"),text_ipv4_dns1);

	lbl_ipv4_dns2=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_ipv4_dns2,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,7,1);
	lv_label_set_text(lbl_ipv4_dns2,_("IPv4 DNS2"));

	text_ipv4_dns2=lv_textarea_create(panel);
	lv_textarea_set_one_line(text_ipv4_dns2,true);
	lv_obj_set_grid_cell(text_ipv4_dns2,LV_GRID_ALIGN_STRETCH,1,3,LV_GRID_ALIGN_STRETCH,7,1);
	inputbox_bind_textarea(_("IPv4 DNS2"),text_ipv4_dns2);

	draw_buttons(panel);
	update_status();
}

void ui_network_panel_ipconfig::switch_to(const std::shared_ptr<device_ipconfig>&dev){
	ui_network_panel_device::switch_to(dev);
	if(current){
		lv_label_set_text_fmt(lbl_title,_("Configure IP address for %s"),current->name.c_str());
		update_status();
		reload();
	}
}

void ui_network_panel_ipconfig::do_back(){
	if(current&&current->on_back)
		current->on_back();
}

bool ui_network_panel_ipconfig::have_back(){
	return current&&current->on_back;
}

ipconfig_profile&ui_network_panel_ipconfig::get_profile(){
	if(!current)throw InvalidArgument("no current device");
	return find_profile(current->name,current->profile);
}

ipconfig_profile&ui_network_panel_ipconfig::find_profile(
	const std::string&dev,
	const std::string&sub
){
	if(dev.empty())throw InvalidArgument("invalid device");
	auto pn=sub.empty()?"default":sub;
	if(!profiles.contains(dev))profiles[dev]={};
	if(!profiles[dev].contains(pn))profiles[dev][pn]={};
	profiles[dev][pn].device=dev;
	profiles[dev][pn].profile=pn;
	return profiles[dev][pn];
}

std::string ipconfig_profile::gen_config(){
	std::string buff{};
	buff+=std::format("interface {}\n",device);
	buff+=std::format("\tipv6\n",device);
	buff+=std::format("\tdhcp6\n",device);
	switch(v4.mode){
		case IPV4_DHCP:
			buff+=std::format("\tipv4\n",device);
			buff+=std::format("\tdhcp\n",device);
		break;
		case IPV4_STATIC:
			buff+=std::format("\tipv4\n",device);
			buff+=std::format("\tnodhcp\n",device);
			if(v4.address.addr)buff+=std::format(
				"\tstatic ip_address={}/{}\n",
				v4.address.addr.to_string(),
				v4.address.mask.prefix
			);
			if(v4.gateway)buff+=std::format(
				"\tstatic routers={}\n",
				v4.gateway.to_string()
			);
		break;
		case IPV4_DISABLED:
			buff+=std::format("\tnoipv4\n",device);
			buff+=std::format("\tnodhcp\n",device);
		break;
	}
	if(!autodns)
		buff+=std::format("nohook resolv.conf\n",device);
	return buff;
}

void ipconfig_profile::configure(){
	std::string dhcpcd{};
	log_info("configure {} use {} with dhcpcd",device,profile);
	if(auto v=config["nanodistro"]["network"]["dhcpcd"])
		dhcpcd=v.as<std::string>();
	if(dhcpcd.empty())dhcpcd=path_find_exec("dhcpcd");
	if(dhcpcd.empty())throw InvalidArgument("dhcpcd not found");
	log_info("dhcpcd found at {}",dhcpcd);
	std::string config{},config_path{};
	size_t id=0;
	do{
		config_path=std::format(
			"/tmp/.nanodistro-{}-dhcpcd-{}-{}-{}.conf",
			getpid(),device,profile,++id
		);
	}while(fs_exists(config_path));
	log_info("create dhcpcd config at {}",config_path);
	config=gen_config();
	log_debug("dhcpcd config: {}",config);
	fs_write_all(config_path,config);
	log_info("killing old dhcpcd...");
	process m_exit{};
	m_exit.set_execute(dhcpcd);
	m_exit.set_command({"dhcpcd","--exit",device});
	m_exit.start();
	m_exit.wait();
	try{
		process m_flush{};
		m_flush.set_command({"ip","address","flush","dev",device});
		m_flush.start();
		m_flush.wait();
	}catch(...){}
	log_info("starting dhcpcd...");
	process m_start{};
	m_start.set_execute(dhcpcd);
	m_start.set_command({"dhcpcd","--config",config_path,device});
	m_start.start();
	m_start.wait();
	unlink(config_path.c_str());
	if(!autodns){
		log_info("configure dns /etc/resolv.conf");
		resolv_conf conf{};
		if(!dns1.empty())conf.nameservers.push_back(dns1);
		if(!dns2.empty())conf.nameservers.push_back(dns2);
		conf.remove();
		conf.save_file();
	}
	log_info("configure {} use {} done",device,profile);
}
