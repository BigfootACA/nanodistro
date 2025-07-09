#include"ui.h"
#include"log.h"

struct inputbox{
	bool use_kbd=true;
	lv_obj_t*kbd=nullptr;
	lv_obj_t*mask=nullptr;
	lv_obj_t*box=nullptr;
	lv_obj_t*lbl=nullptr;
	lv_obj_t*text=nullptr;
	lv_obj_t*btn_ok=nullptr;	
	lv_obj_t*btn_cancel=nullptr;	
	lv_obj_t*tgt_text=nullptr;
};
static inputbox ib{};

void inputbox_set_keyboard_show(bool show){
	lv_obj_set_hidden(ib.kbd,!show);
	if(show){
		auto scr=lv_screen_active();
		auto sh=lv_obj_get_height(scr);
		auto kh=lv_obj_get_height(ib.kbd);
		lv_obj_set_height(ib.mask,sh-kh);
	}else{
		lv_obj_set_height(ib.mask,lv_pct(100));
		lv_keyboard_set_textarea(ib.kbd,NULL);
	}
}

void inputbox_focus_keyboard(){
	auto grp=lv_group_get_default();
	if(ib.use_kbd){
		inputbox_set_keyboard_show(true);
		lv_group_focus_obj(ib.kbd);
		lv_obj_add_state(ib.text,LV_EVENT_FOCUSED);
		lv_obj_remove_state(ib.kbd,LV_EVENT_FOCUSED);
		lv_keyboard_set_textarea(ib.kbd,ib.text);
	}else lv_group_focus_obj(ib.text);
	lv_group_set_editing(grp,true);
}

static void request_close(){
	lv_obj_set_hidden(ib.mask,true);
	lv_obj_set_disabled(current_view,false);
	auto grp=lv_group_get_default();
	lv_group_set_editing(grp,false);
	inputbox_set_keyboard_show(false);
}

static void request_apply_close(lv_event_t*e){
	if(ib.tgt_text){
		auto val=lv_textarea_get_text(ib.text);
		lv_textarea_set_text(ib.tgt_text,val);
	}
	request_close();
}

void inputbox_init(){
	static lv_coord_t grid_cols[]={
		LV_GRID_FR(1),
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};

	ib.mask=lv_create_mask(request_close);
	lv_obj_set_hidden(ib.mask,true);

	ib.box=lv_obj_create(ib.mask);
	lv_obj_set_size(ib.box,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_obj_set_grid_dsc_array(ib.box,grid_cols,grid_rows);
	lv_obj_center(ib.box);

	ib.lbl=lv_label_create(ib.box);
	lv_obj_set_size(ib.lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_obj_set_style_text_align(ib.lbl,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_grid_cell(ib.lbl,LV_GRID_ALIGN_CENTER,0,2,LV_GRID_ALIGN_CENTER,0,1);

	ib.text=lv_textarea_create(ib.box);
	lv_obj_set_grid_cell(ib.text,LV_GRID_ALIGN_CENTER,0,2,LV_GRID_ALIGN_CENTER,1,1);
	lv_obj_add_event_func(ib.text,[](auto){
		auto grp=lv_group_get_default();
		if(!ib.use_kbd||!lv_group_get_editing(grp))return;
		lv_group_set_editing(grp,false);
		inputbox_focus_keyboard();
	},LV_EVENT_FOCUSED);
	lv_obj_add_event_func(ib.text,std::bind(&lv_group_focus_obj,ib.btn_ok),LV_EVENT_READY);
	lv_obj_add_event_func(ib.text,std::bind(&lv_group_focus_obj,ib.btn_cancel),LV_EVENT_CANCEL);

	ib.btn_ok=lv_button_create(ib.box);
	auto lbl_ok=lv_label_create(ib.btn_ok);
	lv_label_set_text(lbl_ok,_("OK"));
	lv_obj_center(lbl_ok);
	lv_obj_set_height(ib.btn_ok,LV_SIZE_CONTENT);
	lv_obj_set_grid_cell(ib.btn_ok,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,2,1);
	lv_obj_add_event_func(ib.btn_ok,request_apply_close,LV_EVENT_CLICKED);

	ib.btn_cancel=lv_button_create(ib.box);
	auto lbl_cancel=lv_label_create(ib.btn_cancel);
	lv_label_set_text(lbl_cancel,_("Cancel"));
	lv_obj_center(lbl_cancel);
	lv_obj_set_height(ib.btn_cancel,LV_SIZE_CONTENT);
	lv_obj_set_grid_cell(ib.btn_cancel,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,2,1);
	lv_obj_add_event_func(ib.btn_cancel,std::bind(request_close),LV_EVENT_CLICKED);

	ib.kbd=lv_keyboard_create(lv_screen_active());
	lv_obj_set_hidden(ib.kbd,true);
	lv_obj_add_event_func(ib.kbd,[](auto e){
		auto k=*(uint32_t*)lv_event_get_param(e);
		if(!isprint(k))return;
		ib.use_kbd=false;
		inputbox_set_keyboard_show(false);
		lv_group_focus_obj(ib.text);
		auto grp=lv_group_get_default();
		lv_group_set_editing(grp,true);
		lv_group_send_data(grp,k);
	},LV_EVENT_KEY);
	auto on_done=[](lv_obj_t*o){
		auto grp=lv_group_get_default();
		lv_group_set_editing(grp,false);
		inputbox_set_keyboard_show(false);
		lv_group_focus_obj(o);
	};
	lv_obj_add_event_func(ib.kbd,std::bind(on_done,ib.btn_ok),LV_EVENT_READY);
	lv_obj_add_event_func(ib.kbd,std::bind(on_done,ib.btn_cancel),LV_EVENT_CANCEL);
}

void inputbox_show(const std::string&msg){
	lv_label_set_text(ib.lbl,msg.c_str());
	lv_obj_set_hidden(ib.mask,false);
	lv_obj_set_disabled(current_view,true);
	inputbox_focus_keyboard();
}

void inputbox_show_textarea(const std::string&msg,lv_obj_t*ta){
	if(!ta)return;
	ib.tgt_text=ta;
	inputbox_show(msg);
	lv_textarea_set_text(ib.text,lv_textarea_get_text(ta));
	lv_textarea_set_cursor_pos(ib.text,lv_textarea_get_cursor_pos(ta));
	lv_textarea_set_one_line(ib.text,lv_textarea_get_one_line(ta));
	lv_textarea_set_password_mode(ib.text,lv_textarea_get_password_mode(ta));
	lv_textarea_set_password_show_time(ib.text,lv_textarea_get_password_show_time(ta));
	lv_textarea_set_max_length(ib.text,lv_textarea_get_max_length(ta));
	lv_textarea_set_accepted_chars(ib.text,lv_textarea_get_accepted_chars(ta));
	lv_textarea_set_text_selection(ib.text,lv_textarea_get_text_selection(ta));
	lv_textarea_set_placeholder_text(ib.text,lv_textarea_get_placeholder_text(ta));
}

void inputbox_bind_textarea(const std::string&msg,lv_obj_t*ta){
	if(!ta)return;
	auto do_show=[msg=std::string(msg),ta](auto){
		inputbox_show_textarea(msg,ta);
	};
	lv_obj_add_event_func(ta,do_show,LV_EVENT_CLICKED);
	lv_obj_add_event_func(ta,[ta,do_show](auto){
		if(ib.tgt_text==ta)return;
		auto grp=lv_group_get_default();
		if(!lv_group_get_editing(grp))return;
		lv_group_set_editing(grp,false);
		lv_async_call_func(std::bind(do_show,nullptr));
	},LV_EVENT_FOCUSED);
	lv_obj_add_event_func(ta,[ta](auto){
		if(ta!=ib.tgt_text)return;
		ib.tgt_text=nullptr;
		request_close();
	},LV_EVENT_DELETE);
	lv_group_add_obj(lv_group_get_default(),ta);
}
