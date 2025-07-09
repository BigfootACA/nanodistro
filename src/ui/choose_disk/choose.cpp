#include"internal.h"
#include"configs.h"
#include"worker.h"
#include"log.h"

std::shared_ptr<ui_draw>ui_create_choose_disk(){
	return std::make_shared<ui_draw_choose_disk>();
}

void ui_draw_choose_disk::draw_buttons(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
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
	lv_obj_set_grid_cell(box,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_END,2,1);

	btn_back=lv_button_create(box);
	auto lbl_btn_back=lv_label_create(btn_back);
	auto f1=std::bind(&ui_draw_choose_disk::btn_back_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_back,f1,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_back,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_back,_("Back"));

	btn_refresh=lv_button_create(box);
	auto lbl_btn_refresh=lv_label_create(btn_refresh);
	auto f2=std::bind(&ui_draw_choose_disk::btn_refresh_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_refresh,f2,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_refresh,LV_GRID_ALIGN_START,1,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_refresh,_("Refresh"));

	btn_next=lv_button_create(box);
	auto lbl_btn_next=lv_label_create(btn_next);
	auto f3=std::bind(&ui_draw_choose_disk::btn_next_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_next,f3,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_next,LV_GRID_ALIGN_END,3,1,LV_GRID_ALIGN_END,0,1);
	lv_obj_set_disabled(btn_next,true);
	lv_label_set_text(lbl_btn_next,_("Next"));
}

void ui_draw_choose_disk::btn_refresh_cb(lv_event_t*){
	set_spinner(true);
	worker_add(std::bind(&ui_draw_choose_disk::load_disks,this));
}

void ui_draw_choose_disk::btn_next_cb(lv_event_t*){
	if(!selected_disk)return;
	installer_context["disk"]=selected_disk->item->to_json();
	ui_switch_page("confirm");
}

void ui_draw_choose_disk::btn_back_cb(lv_event_t*){
	ui_switch_page("choose-image");
}

void ui_draw_choose_disk::draw_spinner(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};

	box_loading=lv_obj_create(cont);
	lv_obj_set_size(box_loading,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_obj_set_style_bg_opa(box_loading,LV_OPA_TRANSP,0);
	lv_obj_set_style_border_width(box_loading,0,0);
	lv_obj_set_style_radius(box_loading,0,0);
	lv_obj_set_grid_dsc_array(box_loading,grid_cols,grid_rows);
	lv_obj_center(box_loading);

	spinner=lv_spinner_create(box_loading);
	lv_obj_set_grid_cell(spinner,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,0,1);

	lbl_loading=lv_label_create(box_loading);
	lv_obj_set_grid_cell(lbl_loading,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_label_set_text(lbl_loading,_("Loading..."));
}

void ui_draw_choose_disk::set_spinner(bool show){
	if(box_loading)lv_obj_delete(box_loading);
	if(mask)lv_obj_delete(mask);
	spinner=nullptr,lbl_loading=nullptr;
	mask=nullptr,box_loading=nullptr;
	if(show){
		mask=lv_create_mask();
		draw_spinner(mask);
	}
	lv_obj_set_disabled(box_info,show);
	lv_obj_set_disabled(lst_disk,show);
	lv_obj_set_disabled(btn_refresh,show);
	lv_obj_set_disabled(btn_next,show||!selected_disk);
}

void ui_draw_choose_disk::draw(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_FR(4),
		LV_GRID_FR(6),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};
	lv_obj_set_grid_dsc_array(cont,grid_cols,grid_rows);

	auto lbl_title=lv_label_create(cont);
	lv_obj_set_style_text_align(lbl_title,LV_TEXT_ALIGN_CENTER,0);
	lv_label_set_text(lbl_title,_("Choose disk to flash"));
	lv_obj_set_grid_cell(lbl_title,LV_GRID_ALIGN_CENTER,0,2,LV_GRID_ALIGN_CENTER,0,1);

	lst_disk=lv_list_create(cont);
	lv_obj_set_grid_cell(lst_disk,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,1,2);

	box_info=lv_obj_create(cont);
	lv_obj_set_grid_cell(box_info,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,1,1);

	lbl_info=lv_label_create(box_info);
	lv_obj_set_style_text_align(lbl_info,LV_TEXT_ALIGN_LEFT,0);
	lv_obj_set_grid_cell(lbl_info,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_obj_set_size(lbl_info,lv_pct(100),LV_SIZE_CONTENT);
	lv_label_set_text(lbl_info,"");

	show_info_placeholder();

	draw_buttons(cont);
	set_spinner(true);
	worker_add(std::bind(&ui_draw_choose_disk::load_disks,this));
}

void ui_draw_choose_disk::clean(){
	lv_image_cache_drop(nullptr);
	set_selected_disk(nullptr);
	disks.clear();
	if(lbl_disk_placeholder){
		lv_obj_delete(lbl_disk_placeholder);
		lbl_disk_placeholder=nullptr;
	}
	lv_obj_clean(lst_disk);
	lv_obj_scroll_to_y(lst_disk,0,LV_ANIM_OFF);
	lv_obj_invalidate(lst_disk);
}

void ui_draw_choose_disk::disk_set_selected(const std::shared_ptr<disk_gui_item>&item,bool s){
	lv_obj_set_grid_cell(item->lbl_size,LV_GRID_ALIGN_END,2,1+(!s),LV_GRID_ALIGN_CENTER,0,1);
	lv_obj_set_grid_cell(item->lbl_subtitle,LV_GRID_ALIGN_STRETCH,1,2+(!s),LV_GRID_ALIGN_CENTER,1,1);
	lv_obj_set_hidden(item->lbl_checked,!s);
	item->selected=s;
}

void ui_draw_choose_disk::set_selected_disk(const std::shared_ptr<disk_gui_item>&item){
	if(!item)lv_label_set_text(lbl_info,"");
	lv_obj_set_disabled(btn_next,!item);
	lv_obj_set_hidden(lbl_info_placeholder,!!item);
	selected_disk=item;
	if(item)show_info(item);
}

void ui_draw_choose_disk::disk_item_click(lv_event_t*ev){
	auto tgt=lv_event_get_target(ev);
	set_selected_disk(nullptr);
	for(auto&disk:disks){
		if(disk->btn_body==tgt){
			disk_set_selected(disk,!disk->selected);
			if(disk->selected)set_selected_disk(disk);
		}else disk_set_selected(disk,false);
	}
}

void ui_draw_choose_disk::show_info_placeholder(){
	if(lbl_info_placeholder){
		lv_obj_set_hidden(lbl_info_placeholder,false);
	}else{
		lbl_info_placeholder=lv_label_create(box_info);
		lv_obj_set_style_text_align(lbl_info_placeholder,LV_TEXT_ALIGN_CENTER,0);
		lv_obj_set_size(lbl_info_placeholder,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
		lv_label_set_text(lbl_info_placeholder,_("No disk selected"));
		lv_obj_center(lbl_info_placeholder);
	}
}

void ui_draw_choose_disk::show_disk_placeholder(){
	if(lbl_disk_placeholder){
		lv_obj_set_hidden(lbl_disk_placeholder,false);
	}else{
		lbl_disk_placeholder=lv_label_create(lst_disk);
		lv_obj_set_style_text_align(lbl_disk_placeholder,LV_TEXT_ALIGN_CENTER,0);
		lv_obj_set_size(lbl_disk_placeholder,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
		lv_label_set_text(lbl_disk_placeholder,_("No disk found"));
		lv_obj_center(lbl_disk_placeholder);
	}
}
