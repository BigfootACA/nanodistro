#include"ui.h"
#include"log.h"

struct msgbox{
	lv_obj_t*mask=nullptr;
	lv_obj_t*box=nullptr;
	lv_obj_t*lbl=nullptr;
	lv_obj_t*btn_ok=nullptr;
};
static msgbox mb{};

static void request_close(){
	lv_obj_set_hidden(mb.mask,true);
	lv_obj_set_disabled(current_view,false);
}

void msgbox_init(){
	static lv_coord_t grid_cols[]={
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};

	mb.mask=lv_create_mask(request_close);
	lv_obj_set_hidden(mb.mask,true);

	mb.box=lv_obj_create(mb.mask);
	lv_obj_set_size(mb.box,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_obj_set_grid_dsc_array(mb.box,grid_cols,grid_rows);
	lv_obj_center(mb.box);

	mb.lbl=lv_label_create(mb.box);
	lv_obj_set_size(mb.lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_obj_set_style_text_align(mb.lbl,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_grid_cell(mb.lbl,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,0,1);

	mb.btn_ok=lv_button_create(mb.box);
	auto lbl_ok=lv_label_create(mb.btn_ok);
	lv_label_set_text(lbl_ok,_("OK"));
	lv_obj_center(lbl_ok);
	lv_obj_set_size(mb.btn_ok,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_obj_set_grid_cell(mb.btn_ok,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_obj_add_event_func(mb.btn_ok,std::bind(request_close),LV_EVENT_CLICKED);

	lv_obj_update_layout(mb.mask);
	lv_obj_set_style_max_width(mb.box,lv_obj_get_content_width(mb.mask),0);
	lv_obj_set_style_max_width(mb.lbl,lv_obj_get_content_width(mb.mask)*3/4,0);
}

void msgbox_show(const std::string&msg){
	lv_label_set_text(mb.lbl,msg.c_str());
	lv_obj_set_hidden(mb.mask,false);
	lv_obj_set_disabled(current_view,true);
	lv_group_focus_obj(mb.btn_ok);
}
