#include<csignal>
#include"internal.h"
#include"log.h"

void ui_draw_progress::draw_buttons(){
	static lv_coord_t grid_cols[]={
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};

	auto buttons=lv_obj_create(box);
	lv_obj_set_size(buttons,lv_pct(100),LV_SIZE_CONTENT);
	lv_obj_set_style_radius(buttons,0,0);
	lv_obj_set_style_pad_all(buttons,lv_dpx(6),0);
	lv_obj_set_style_bg_opa(buttons,0,0);
	lv_obj_set_style_border_width(buttons,0,0);
	lv_obj_set_grid_dsc_array(buttons,grid_cols,grid_rows);
	lv_obj_set_grid_cell(buttons,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,4,1);

	btn_cancel=lv_button_create(buttons);
	auto lbl_btn_cancel=lv_label_create(btn_cancel);
	lv_obj_set_grid_cell(btn_cancel,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_cancel,_("Cancel"));
	auto f1=std::bind(&ui_draw_progress::btn_cancel_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_cancel,f1,LV_EVENT_CLICKED);

	btn_back=lv_button_create(buttons);
	auto lbl_btn_back=lv_label_create(btn_back);
	lv_obj_set_grid_cell(btn_back,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_back,_("Back"));
	auto f2=std::bind(&ui_draw_progress::btn_back_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_back,f2,LV_EVENT_CLICKED);

	btn_log=lv_button_create(buttons);
	auto lbl_btn_log=lv_label_create(btn_log);
	lv_obj_set_grid_cell(btn_log,LV_GRID_ALIGN_CENTER,2,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_log,_("Log"));
	auto f3=std::bind(&ui_draw_progress::btn_log_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_log,f3,LV_EVENT_CLICKED);
}

void ui_draw_progress::btn_cancel_cb(lv_event_t*){
	if(state!=STATE_RUNNING)return;
	if(pty_pid>0)kill(pty_pid,SIGTERM);
	set_state(STATE_CANCEL);
}

void ui_draw_progress::btn_log_cb(lv_event_t*){
	set_termview_show(!termview_show);
}

void ui_draw_progress::btn_back_cb(lv_event_t*){
	ui_switch_page("hello");
}

void ui_draw_progress::test(lv_timer_t*){
	static int i=0;
	if(i==0)set_progress(true,0);
	if(i==1)set_progress(true,33);
	if(i==2)set_progress(true,66);
	if(i==3)set_progress(true,100);
	if(i==4)set_progress(false,0);
	if(i==5)set_state(STATE_SUCCESS);
	if(i==6)set_state(STATE_FAILED);
	if(i==7)set_state(STATE_RUNNING);
	i++,i%=8;
}
