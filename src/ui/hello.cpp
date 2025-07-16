#include"ui.h"
#include"log.h"
#include<lvgl.h>

class ui_draw_hello:public ui_draw{
	public:
		void draw(lv_obj_t*cont)override;
		void btn_start_cb(lv_event_t*ev);
		void btn_reboot_cb(lv_event_t*ev);
		void btn_shutdown_cb(lv_event_t*ev);
		void btn_mass_storage_cb(lv_event_t*ev);
		void btn_terminal_cb(lv_event_t*ev);
};

std::shared_ptr<ui_draw>ui_create_hello(){
	return std::make_shared<ui_draw_hello>();
}

void ui_draw_hello::btn_start_cb(lv_event_t*ev){
	ui_switch_page("network");
}

void ui_draw_hello::btn_reboot_cb(lv_event_t*ev){
}

void ui_draw_hello::btn_shutdown_cb(lv_event_t*ev){
}

void ui_draw_hello::btn_mass_storage_cb(lv_event_t*ev){
	ui_switch_page("mass-storage");
}

void ui_draw_hello::btn_terminal_cb(lv_event_t*ev){
	display_switch_tty(3);
}

void ui_draw_hello::draw(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};

	auto obj=lv_obj_create(cont);
	lv_obj_align(obj,LV_ALIGN_CENTER,0,0);
	lv_obj_set_size(obj,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_obj_set_grid_dsc_array(obj,grid_cols,grid_rows);

	auto lbl_title=lv_label_create(obj);
	lv_obj_set_style_text_align(lbl_title,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_size(lbl_title,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_label_set_text(lbl_title,_("Welcome to Distro flash wizard"));
	lv_obj_set_grid_cell(lbl_title,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,0,1);

	auto btn_start=lv_button_create(obj);
	auto lbl_btn_start=lv_label_create(btn_start);
	lv_label_set_text(lbl_btn_start,_("Start"));
	lv_obj_set_grid_cell(btn_start,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,1,1);
	auto f1=std::bind(&ui_draw_hello::btn_start_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_start,f1,LV_EVENT_CLICKED);
	lv_obj_center(lbl_btn_start);
	lv_group_focus_obj(btn_start);

	auto btn_mass_storage=lv_button_create(obj);
	auto lbl_btn_mass_storage=lv_label_create(btn_mass_storage);
	lv_label_set_text(lbl_btn_mass_storage,_("USB Mass Storage"));
	lv_obj_set_grid_cell(btn_mass_storage,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,2,1);
	auto f2=std::bind(&ui_draw_hello::btn_mass_storage_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_mass_storage,f2,LV_EVENT_CLICKED);
	lv_obj_center(lbl_btn_mass_storage);
	lv_group_focus_obj(btn_mass_storage);

	auto btn_terminal=lv_button_create(obj);
	auto lbl_btn_terminal=lv_label_create(btn_terminal);
	lv_label_set_text(lbl_btn_terminal,_("Terminal"));
	lv_obj_set_grid_cell(btn_terminal,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,3,1);
	auto f3=std::bind(&ui_draw_hello::btn_terminal_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_terminal,f3,LV_EVENT_CLICKED);
	lv_obj_center(lbl_btn_terminal);
	lv_group_focus_obj(btn_terminal);

	auto btn_reboot=lv_button_create(obj);
	auto lbl_btn_reboot=lv_label_create(btn_reboot);
	lv_label_set_text(lbl_btn_reboot,_("Reboot"));
	lv_obj_set_grid_cell(btn_reboot,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,4,1);
	auto f4=std::bind(&ui_draw_hello::btn_reboot_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_reboot,f4,LV_EVENT_CLICKED);
	lv_obj_set_style_bg_color(btn_reboot,lv_color_make(100,100,100),0);
	lv_obj_center(lbl_btn_reboot);

	auto btn_shutdown=lv_button_create(obj);
	auto lbl_btn_shutdown=lv_label_create(btn_shutdown);
	lv_label_set_text(lbl_btn_shutdown,_("Shutdown"));
	lv_obj_set_grid_cell(btn_shutdown,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,5,1);
	auto f5=std::bind(&ui_draw_hello::btn_shutdown_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_shutdown,f5,LV_EVENT_CLICKED);
	lv_obj_set_style_bg_color(btn_shutdown,lv_color_make(100,100,100),0);
	lv_obj_center(lbl_btn_shutdown);
}
