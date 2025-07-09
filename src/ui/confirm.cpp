#include"ui.h"
#include"log.h"
#include"configs.h"
#include"json-utils.h"
#include<lvgl.h>

class ui_draw_confirm:public ui_draw{
	public:
		void draw(lv_obj_t*cont)override;
		void btn_continue_cb(lv_event_t*ev);
		void btn_back_cb(lv_event_t*ev);
		void btn_cancel_cb(lv_event_t*ev);
};

std::shared_ptr<ui_draw>ui_create_confirm(){
	return std::make_shared<ui_draw_confirm>();
}

void ui_draw_confirm::btn_continue_cb(lv_event_t*ev){
	installer_context["config"]={Json::objectValue};
	yaml_to_json(installer_context["config"],config);
	ui_switch_page("progress");
}

void ui_draw_confirm::btn_back_cb(lv_event_t*ev){
	ui_switch_page("choose-disk");
}

void ui_draw_confirm::btn_cancel_cb(lv_event_t*ev){
	ui_switch_page("hello");
}

void ui_draw_confirm::draw(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
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
	auto diskdev=installer_context["disk"]["devname"].asString();
	lv_label_set_text_fmt(lbl_title,_(
		"CAUTION:\n"
		"You are flashing a new image to the disk.\n"
		"The installer may erase all data on the disk.\n"
		"Are you sure you want to continue?\n"
		"Target Disk: %s"
	),diskdev.c_str());
	lv_obj_set_grid_cell(lbl_title,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,0,1);

	auto btn_continue=lv_button_create(obj);
	auto lbl_btn_continue=lv_label_create(btn_continue);
	lv_label_set_text(lbl_btn_continue,_("Continue"));
	lv_obj_set_grid_cell(btn_continue,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,1,1);
	auto f1=std::bind(&ui_draw_confirm::btn_continue_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_continue,f1,LV_EVENT_CLICKED);
	lv_obj_center(lbl_btn_continue);
	lv_group_focus_obj(btn_continue);

	auto btn_back=lv_button_create(obj);
	auto lbl_btn_back=lv_label_create(btn_back);
	lv_label_set_text(lbl_btn_back,_("Back"));
	lv_obj_set_grid_cell(btn_back,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,2,1);
	auto f2=std::bind(&ui_draw_confirm::btn_back_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_back,f2,LV_EVENT_CLICKED);
	lv_obj_set_style_bg_color(btn_back,lv_color_make(100,100,100),0);
	lv_obj_center(lbl_btn_back);

	auto btn_cancel=lv_button_create(obj);
	auto lbl_btn_cancel=lv_label_create(btn_cancel);
	lv_label_set_text(lbl_btn_cancel,_("Cancel"));
	lv_obj_set_grid_cell(btn_cancel,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,3,1);
	auto f3=std::bind(&ui_draw_confirm::btn_cancel_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_cancel,f3,LV_EVENT_CLICKED);
	lv_obj_set_style_bg_color(btn_cancel,lv_color_make(100,100,100),0);
	lv_obj_center(lbl_btn_cancel);
}
