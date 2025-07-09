#include"log.h"
#include"configs.h"
#include"network.h"

void ui_network_panel_proxy::show(){
	ui_network_panel::show();
	lv_group_focus_obj(btn_apply);
}

void ui_network_panel_proxy::reload(){
	lv_textarea_set_text(all_proxy,"");
	lv_textarea_set_text(ftp_proxy,"");
	lv_textarea_set_text(http_proxy,"");
	lv_textarea_set_text(https_proxy,"");
	lv_textarea_set_text(no_proxy,"");
	if(auto val=getenv("ALL_PROXY"))
		lv_textarea_set_text(all_proxy,val);
	if(auto val=getenv("FTP_PROXY"))
		lv_textarea_set_text(ftp_proxy,val);
	if(auto val=getenv("HTTP_PROXY"))
		lv_textarea_set_text(http_proxy,val);
	if(auto val=getenv("HTTPS_PROXY"))
		lv_textarea_set_text(https_proxy,val);
	if(auto val=getenv("NO_PROXY"))
		lv_textarea_set_text(no_proxy,val);
	if(auto val=getenv("all_proxy"))
		lv_textarea_set_text(all_proxy,val);
	if(auto val=getenv("ftp_proxy"))
		lv_textarea_set_text(ftp_proxy,val);
	if(auto val=getenv("http_proxy"))
		lv_textarea_set_text(http_proxy,val);
	if(auto val=getenv("https_proxy"))
		lv_textarea_set_text(https_proxy,val);
	if(auto val=getenv("no_proxy"))
		lv_textarea_set_text(no_proxy,val);
}

void ui_network_panel_proxy::clear(){
	unsetenv("ALL_PROXY");
	unsetenv("FTP_PROXY");
	unsetenv("HTTP_PROXY");
	unsetenv("HTTPS_PROXY");
	unsetenv("NO_PROXY");
	unsetenv("all_proxy");
	unsetenv("ftp_proxy");
	unsetenv("http_proxy");
	unsetenv("https_proxy");
	unsetenv("no_proxy");
}

void ui_network_panel_proxy::btn_clear_clicked(lv_event_t*){
	clear();
}

void ui_network_panel_proxy::btn_apply_clicked(lv_event_t*){
	auto all_proxy_str=lv_textarea_get_text(all_proxy);
	auto ftp_proxy_str=lv_textarea_get_text(ftp_proxy);
	auto http_proxy_str=lv_textarea_get_text(http_proxy);
	auto https_proxy_str=lv_textarea_get_text(https_proxy);
	auto no_proxy_str=lv_textarea_get_text(no_proxy);
	clear();
	if(all_proxy_str&&*all_proxy_str){
		setenv("ALL_PROXY",all_proxy_str,1);
		setenv("all_proxy",all_proxy_str,1);
	}
	if(ftp_proxy_str&&*ftp_proxy_str){
		setenv("FTP_PROXY",ftp_proxy_str,1);
		setenv("ftp_proxy",ftp_proxy_str,1);
	}
	if(http_proxy_str&&*http_proxy_str){
		setenv("HTTP_PROXY",http_proxy_str,1);
		setenv("http_proxy",http_proxy_str,1);
	}
	if(https_proxy_str&&*https_proxy_str){
		setenv("HTTPS_PROXY",https_proxy_str,1);
		setenv("https_proxy",https_proxy_str,1);
	}
	if(no_proxy_str&&*no_proxy_str){
		setenv("NO_PROXY",no_proxy_str,1);
		setenv("no_proxy",no_proxy_str,1);
	}
}

void ui_network_panel_proxy::btn_proxy_clicked(lv_event_t*){
	show();
	lv_obj_add_state(btn_panel_proxy,LV_STATE_CHECKED);
}

void ui_network_panel_proxy::draw_buttons(lv_obj_t*cont){
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
	lv_obj_set_grid_cell(box,LV_GRID_ALIGN_STRETCH,0,2,LV_GRID_ALIGN_END,7,1);

	btn_apply=lv_button_create(box);
	auto lbl_btn_apply=lv_label_create(btn_apply);
	lv_obj_set_grid_cell(btn_apply,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_btn_apply,_("Apply"));
	auto f1=std::bind(&ui_network_panel_proxy::btn_apply_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_apply,f1,LV_EVENT_CLICKED);

	btn_clear=lv_button_create(box);
	auto lbl_btn_clear=lv_label_create(btn_clear);
	lv_obj_set_grid_cell(btn_clear,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_btn_clear,_("Clear"));
	auto f2=std::bind(&ui_network_panel_proxy::btn_clear_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_clear,f2,LV_EVENT_CLICKED);
}

void ui_network_panel_proxy::draw(lv_obj_t*obj){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
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

	auto lbl_title=lv_label_create(panel);
	lv_obj_set_style_margin_all(lbl_title,28,0);
	lv_obj_set_grid_cell(lbl_title,LV_GRID_ALIGN_CENTER,0,2,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_title,_("Setup Proxy"));

	auto lbl_all_proxy=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_all_proxy,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_label_set_text(lbl_all_proxy,_("All proxy"));

	all_proxy=lv_textarea_create(panel);
	lv_textarea_set_one_line(all_proxy,true);
	lv_obj_set_grid_cell(all_proxy,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,1,1);
	inputbox_bind_textarea(_("All proxy"),all_proxy);

	auto lbl_ftp_proxy=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_ftp_proxy,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,2,1);
	lv_label_set_text(lbl_ftp_proxy,_("FTP proxy"));

	ftp_proxy=lv_textarea_create(panel);
	lv_textarea_set_one_line(ftp_proxy,true);
	lv_obj_set_grid_cell(ftp_proxy,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,2,1);
	inputbox_bind_textarea(_("FTP proxy"),ftp_proxy);

	auto lbl_http_proxy=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_http_proxy,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,3,1);
	lv_label_set_text(lbl_http_proxy,_("HTTP proxy"));

	http_proxy=lv_textarea_create(panel);
	lv_textarea_set_one_line(http_proxy,true);
	lv_obj_set_grid_cell(http_proxy,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,3,1);
	inputbox_bind_textarea(_("HTTP proxy"),http_proxy);

	auto lbl_https_proxy=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_https_proxy,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,4,1);
	lv_label_set_text(lbl_https_proxy,_("HTTPS proxy"));

	https_proxy=lv_textarea_create(panel);
	lv_textarea_set_one_line(https_proxy,true);
	lv_obj_set_grid_cell(https_proxy,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,4,1);
	inputbox_bind_textarea(_("HTTPS proxy"),https_proxy);

	auto lbl_no_proxy=lv_label_create(panel);
	lv_obj_set_grid_cell(lbl_no_proxy,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,5,1);
	lv_label_set_text(lbl_no_proxy,_("Not proxy for"));

	no_proxy=lv_textarea_create(panel);
	lv_textarea_set_one_line(no_proxy,true);
	lv_obj_set_grid_cell(no_proxy,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,5,1);
	inputbox_bind_textarea(_("Not proxy for"),no_proxy);

	draw_buttons(panel);
}

void ui_network_panel_proxy::draw_setting(lv_obj_t*lst){
	if(auto v=config["nanodistro"]["network"]["proxy"]["enabled"];!v||!v.as<bool>())return;
	btn_panel_proxy=lv_list_add_button_ex(lst,"\xf3\xb0\x92\x8d",_("Proxy")); /* mdi-server-network */
	lv_obj_add_flag(btn_panel_proxy,LV_OBJ_FLAG_CHECKABLE);
	auto f=std::bind(&ui_network_panel_proxy::btn_proxy_clicked,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_panel_proxy,f,LV_EVENT_CLICKED);
}
