#include"internal.h"
#include"configs.h"
#include"worker.h"
#include"log.h"

std::shared_ptr<ui_draw>ui_create_choose_image(){
	return std::make_shared<ui_draw_choose_image>();
}

void ui_draw_choose_image::draw_buttons(lv_obj_t*cont){
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
	auto f1=std::bind(&ui_draw_choose_image::btn_back_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_back,f1,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_back,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_back,_("Back"));

	btn_refresh=lv_button_create(box);
	auto lbl_btn_refresh=lv_label_create(btn_refresh);
	auto f2=std::bind(&ui_draw_choose_image::btn_refresh_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_refresh,f2,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_refresh,LV_GRID_ALIGN_START,1,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_refresh,_("Refresh"));

	btn_next=lv_button_create(box);
	auto lbl_btn_next=lv_label_create(btn_next);
	auto f3=std::bind(&ui_draw_choose_image::btn_next_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_next,f3,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_next,LV_GRID_ALIGN_END,3,1,LV_GRID_ALIGN_END,0,1);
	lv_obj_set_disabled(btn_next,true);
	lv_label_set_text(lbl_btn_next,_("Next"));
}

void ui_draw_choose_image::btn_refresh_cb(lv_event_t*){
	set_spinner(true);
	worker_add(std::bind(&ui_draw_choose_image::load_images,this));
}

void ui_draw_choose_image::btn_next_cb(lv_event_t*){
	if(!selected_image)return;
	installer_context["images-url"]=images_url.to_string();
	installer_context["image"]=selected_image->data;
	ui_switch_page("choose-disk");
}

void ui_draw_choose_image::btn_back_cb(lv_event_t*){
	ui_switch_page("network");
}

void ui_draw_choose_image::draw_spinner(lv_obj_t*cont){
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

void ui_draw_choose_image::set_spinner(bool show){
	if(box_loading)lv_obj_delete(box_loading);
	if(mask)lv_obj_delete(mask);
	spinner=nullptr,lbl_loading=nullptr;
	mask=nullptr,box_loading=nullptr;
	if(show){
		mask=lv_create_mask();
		draw_spinner(mask);
	}
	lv_obj_set_disabled(lst_group,show);
	lv_obj_set_disabled(lst_image,show);
	lv_obj_set_disabled(btn_refresh,show);
	lv_obj_set_disabled(btn_next,show||!selected_image);
}

void ui_draw_choose_image::draw(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_FR(35),
		LV_GRID_FR(65),
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
	lv_label_set_text(lbl_title,_("Choose image to flash"));
	lv_obj_set_grid_cell(lbl_title,LV_GRID_ALIGN_CENTER,0,2,LV_GRID_ALIGN_CENTER,0,1);

	lst_group=lv_list_create(cont);
	lv_obj_set_grid_cell(lst_group,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,1,2);

	lst_image=lv_list_create(cont);
	lv_obj_set_grid_cell(lst_image,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,1,1);

	draw_buttons(cont);
	set_spinner(true);
	worker_add(std::bind(&ui_draw_choose_image::load_images,this));
}

void ui_draw_choose_image::clean(){
	lv_image_cache_drop(nullptr);
	for(auto&g:groups){
		g->img_cover=nullptr;
		g->btn_body=nullptr;
	}
	for(auto&i:images){
		i->img_cover=nullptr;
		i->btn_body=nullptr;
	}
	set_selected_image(nullptr);
	groups.clear();
	images.clear();
	lbl_group_placeholder=nullptr;
	lbl_image_placeholder=nullptr;
	lv_obj_set_style_layout(lst_group,LV_LAYOUT_FLEX,0);
	lv_obj_set_style_layout(lst_image,LV_LAYOUT_FLEX,0);
	lv_obj_clean(lst_group);
	lv_obj_clean(lst_image);
	lv_obj_scroll_to_y(lst_group,0,LV_ANIM_OFF);
	lv_obj_scroll_to_y(lst_image,0,LV_ANIM_OFF);
	lv_obj_invalidate(lst_group);
	lv_obj_invalidate(lst_image);
}
