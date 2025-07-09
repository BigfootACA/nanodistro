#include"ui.h"
#include"log.h"
#include"network.h"
#include"configs.h"
#include<lvgl.h>

std::shared_ptr<ui_draw>ui_create_network(){
	return std::make_shared<ui_draw_network>();
}

void ui_network_panel::hide(){
	lv_obj_set_hidden(panel,true);
	lv_obj_set_disabled(panel,true);
}

void ui_draw_network::hide_all(){
	for(auto&p:panels)p->hide();
}

void ui_network_panel::show(){
	net->current=this;
	for(auto&p:net->panels)
		if(p!=this)p->hide();
	lv_obj_set_hidden(panel,false);
	lv_obj_set_disabled(panel,false);
	lv_obj_set_disabled(net->btn_back,!have_back());
	lv_obj_scroll_to_y(net->panel,0,LV_ANIM_OFF);
}

void ui_network_panel::draw(lv_obj_t*obj){
	panel=lv_obj_create(obj);
	lv_obj_set_size(panel,lv_pct(100),LV_SIZE_CONTENT);
	lv_obj_set_style_radius(panel,0,0);
	lv_obj_set_style_bg_opa(panel,0,0);
	lv_obj_set_style_border_width(panel,0,0);
}

void ui_draw_network::btn_back_cb(lv_event_t*ev){
	if(current&&current->have_back())
		current->do_back();
}

void ui_draw_network::btn_skip_cb(lv_event_t*ev){
	ui_switch_page("choose-image");
}

void ui_draw_network::btn_next_cb(lv_event_t*ev){
	installer_context["network"]["configured"]=true;
	ui_switch_page("choose-image");
}

void ui_draw_network::draw_buttons(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
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
	lv_obj_set_grid_cell(box,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_END,1,1);

	btn_back=lv_button_create(box);
	auto lbl_btn_back=lv_label_create(btn_back);
	lv_obj_set_grid_cell(btn_back,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_END,0,1);
	lv_obj_set_disabled(btn_back,true);
	lv_label_set_text(lbl_btn_back,_("Back"));
	auto f1=std::bind(&ui_draw_network::btn_back_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_back,f1,LV_EVENT_CLICKED);

	btn_skip=lv_button_create(box);
	auto lbl_btn_skip=lv_label_create(btn_skip);
	lv_obj_set_grid_cell(btn_skip,LV_GRID_ALIGN_END,1,1,LV_GRID_ALIGN_END,0,1);
	auto f2=std::bind(&ui_draw_network::btn_skip_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_skip,f2,LV_EVENT_CLICKED);
	lv_label_set_text(lbl_btn_skip,_("Skip"));
	lv_group_focus_obj(btn_skip);

	btn_next=lv_button_create(box);
	auto lbl_btn_next=lv_label_create(btn_next);
	lv_obj_set_grid_cell(btn_next,LV_GRID_ALIGN_END,3,1,LV_GRID_ALIGN_END,0,1);
	auto f3=std::bind(&ui_draw_network::btn_next_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_next,f3,LV_EVENT_CLICKED);
	lv_label_set_text(lbl_btn_next,_("Next"));

	auto configured=installer_context["network"]["configured"].asBool();
	lv_obj_set_disabled(btn_next,!configured);
}

void ui_draw_network::draw(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_FR(3),
		LV_GRID_FR(7),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};
	lv_obj_set_grid_dsc_array(cont,grid_cols,grid_rows);

	auto lst_menu=lv_list_create(cont);
	lv_obj_set_grid_cell(lst_menu,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,0,2);
	lv_list_add_text(lst_menu,_("Devices"));
	for(auto&p:panels)p->draw_device(lst_menu);
	lv_list_add_text(lst_menu,_("Settings"));
	for(auto&p:panels)p->draw_setting(lst_menu);

	panel=lv_obj_create(cont);
	lv_obj_set_style_pad_all(panel,0,0);
	lv_obj_set_grid_cell(panel,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,0,1);

	for(auto&p:panels)p->draw(panel);
	hide_all();
	draw_buttons(cont);
	hello.show();
}
