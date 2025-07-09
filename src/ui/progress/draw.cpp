#include"internal.h"
#include"log.h"
#include"gui.h"

std::shared_ptr<ui_draw>ui_create_progress(){
	return std::make_shared<ui_draw_progress>();
}

void ui_draw_progress::set_termview_show(bool show){
	lv_obj_set_hidden(termview,!show);
	if(show){
		lv_obj_set_height(box,lv_pct(50));
		lv_async_call_func(std::bind(lv_termview_update,termview));
	}else{
		lv_obj_set_height(box,lv_pct(100));
	}
	termview_show=show;
}

void ui_draw_progress::set_state(progress_state state){
	switch(state){
		case STATE_RUNNING:
			set_progress(false,0);
			lv_label_set_text(lbl_status,_("Installing..."));
		break;
		case STATE_SUCCESS:
			set_progress(true,100);
			lv_label_set_text(lbl_status,_("Install complete"));
		break;
		case STATE_FAILED:
			set_progress(true,0);
			lv_label_set_text(lbl_status,_("Install failed"));
		break;
		case STATE_CANCEL:
			lv_label_set_text(lbl_status,_("Cancelling..."));
		break;
	}
	lv_obj_set_disabled(btn_cancel,state==STATE_CANCEL);
	lv_obj_set_hidden(lbl_progress,true);
	lv_obj_set_hidden(lbl_success,state!=STATE_SUCCESS);
	lv_obj_set_hidden(lbl_failed,state!=STATE_FAILED);
	lv_obj_set_hidden(btn_back,state==STATE_RUNNING);
	lv_obj_set_hidden(btn_cancel,state!=STATE_RUNNING);
	lv_label_set_text(lbl_report,"");
	this->state=state;
}

void ui_draw_progress::set_progress(bool en,int val){
	val=std::clamp(val,0,100);
	if(progress_enable!=en){
		lv_obj_set_hidden(lbl_progress,!en);
		if(en){
			lv_anim_delete(spinner,nullptr);
			lv_arc_set_start_angle(spinner,0);
			lv_arc_set_end_angle(spinner,progress_value*360/100);
		}else{
			lv_spinner_set_anim_params(spinner,1000,200);
		}
		progress_enable=en;
	}
	if(val!=progress_value){
		progress_value=val;
		lv_label_set_text_fmt(lbl_progress,"%d",val);
		if(progress_enable)
			lv_arc_set_end_angle(spinner,val*360/100);
	}
	lv_obj_align_to(lbl_progress,spinner,LV_ALIGN_CENTER,0,0);
}

void ui_draw_progress::draw(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	};

	lv_obj_set_style_pad_all(cont,0,0);

	box=lv_obj_create(cont);
	lv_obj_align(box,LV_ALIGN_TOP_MID,0,0);
	lv_obj_set_style_bg_opa(box,LV_OPA_TRANSP,0);
	lv_obj_set_style_border_width(box,0,0);
	lv_obj_set_style_radius(box,0,0);
	lv_obj_set_size(box,lv_pct(100),lv_pct(100));
	lv_obj_set_grid_dsc_array(box,grid_cols,grid_rows);
	lv_obj_set_scrollbar_mode(box,LV_SCROLLBAR_MODE_OFF);

	spinner=lv_spinner_create(box);
	lv_obj_set_grid_cell(spinner,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,1,1);

	lbl_progress=lv_label_create(box);
	lv_obj_set_style_text_align(lbl_progress,LV_TEXT_ALIGN_CENTER,0);
	lv_label_set_text(lbl_progress,"0");
	auto font=fonts_get("ui-large");
	if(font)lv_obj_set_style_text_font(lbl_progress,font,0);
	lv_obj_set_grid_cell(lbl_progress,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_obj_set_hidden(lbl_progress,true);

	auto mdi=fonts_get("mdi-large");
	lbl_success=lv_label_create(box);
	lv_obj_set_style_text_align(lbl_success,LV_TEXT_ALIGN_CENTER,0);
	lv_label_set_text(lbl_success,"\xf3\xb0\x84\xac"); /* mdi-check */
	if(mdi)lv_obj_set_style_text_font(lbl_success,mdi,0);
	lv_obj_set_grid_cell(lbl_success,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_obj_set_hidden(lbl_success,true);

	lbl_failed=lv_label_create(box);
	lv_obj_set_style_text_align(lbl_failed,LV_TEXT_ALIGN_CENTER,0);
	lv_label_set_text(lbl_failed,"\xf3\xb0\x85\x96"); /* mdi-close */
	if(mdi)lv_obj_set_style_text_font(lbl_failed,mdi,0);
	lv_obj_set_grid_cell(lbl_failed,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_obj_set_hidden(lbl_failed,true);

	lbl_status=lv_label_create(box);
	lv_obj_set_style_text_align(lbl_status,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_grid_cell(lbl_status,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,2,1);
	lv_label_set_text(lbl_status,_("Installing..."));

	lbl_report=lv_label_create(box);
	lv_obj_set_style_text_align(lbl_report,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_grid_cell(lbl_report,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,3,1);
	lv_label_set_text(lbl_report,"Realtime Status");

	draw_buttons();

	termview=lv_termview_create(cont);
	lv_obj_set_size(termview,lv_pct(100),lv_pct(50));
	lv_obj_align(termview,LV_ALIGN_BOTTOM_MID,0,0);
	init_terminal();

	set_state(STATE_RUNNING);
	set_termview_show(false);
	start_terminal();
}

ui_draw_progress::~ui_draw_progress(){
	stop_terminal();
}
