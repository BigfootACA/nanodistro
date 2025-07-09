#include"internal.h"
#include"builtin.h"
#include"worker.h"
#include"gui.h"

void ui_draw_choose_image::group_set_selected(const std::shared_ptr<group_item>&item,bool selected){
	auto s=selected?1:2;
	lv_obj_set_grid_cell(item->lbl_title,LV_GRID_ALIGN_STRETCH,1,s,LV_GRID_ALIGN_CENTER,0,1);
	lv_obj_set_hidden(item->lbl_checked,!selected);
	item->selected=selected;
}

void ui_draw_choose_image::group_item_click(lv_event_t*ev){
	auto tgt=lv_event_get_target(ev);
	std::shared_ptr<group_item>item=nullptr;
	for(auto&grp:groups){
		if(grp->btn_body==tgt){
			group_set_selected(grp,!grp->selected);
			item=grp;
		}else group_set_selected(grp,false);
	}
	size_t images_show=0;
	for(auto&img:images){
		bool show=true;
		auto grp_id=img->data["group"].asString();
		if(item&&item->selected){
			if(grp_id.empty())show=false;
			else if(grp_id!=item->id)show=false;
		}
		lv_obj_set_hidden(img->btn_body,!show);
		if(show)images_show++;
	}
	if(lbl_image_placeholder){
		lv_obj_set_hidden(lbl_image_placeholder,images_show>0);
		lv_obj_set_style_layout(lst_image,images_show>0?LV_LAYOUT_FLEX:LV_LAYOUT_NONE,0);
	}
	if(selected_image){
		image_set_selected(selected_image,false);
		set_selected_image(nullptr);
	}
	lv_obj_scroll_to_y(lst_image,0,LV_ANIM_OFF);
	lv_obj_invalidate(lst_image);
}

void ui_draw_choose_image::populate_group(const std::shared_ptr<group_item>&item){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};
	item->btn_body=lv_obj_class_create_obj(&lv_list_button_class,lst_group);
	lv_obj_class_init_obj(item->btn_body);
	lv_obj_set_grid_dsc_array(item->btn_body,grid_cols,grid_rows);
	auto f1=std::bind(&ui_draw_choose_image::group_item_click,this,std::placeholders::_1);
	lv_obj_add_event_func(item->btn_body,f1,LV_EVENT_CLICKED);
	auto name=item->data["name"].asString();

	auto s=lv_dpx(48);
	item->img_width=s;
	item->img_dsc.header.w=s;
	item->img_dsc.header.h=s;
	item->img_dsc.header.cf=LV_COLOR_FORMAT_ARGB8888;
	item->img_dsc.header.magic=LV_IMAGE_HEADER_MAGIC;
	item->img_dsc.header.stride=s*sizeof(lv_color32_t);
	extern builtin_file file_svg_default;
	item->img_dsc.data=(const uint8_t*)file_svg_default.data;
	item->img_dsc.data_size=file_svg_default.size;
	item->img_cover=lv_image_create(item->btn_body);
	lv_obj_set_size(item->img_cover,s,s);
	lv_image_set_src(item->img_cover,&item->img_dsc);
	lv_obj_set_grid_cell(item->img_cover,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,0,1);

	item->lbl_title=lv_label_create(item->btn_body);
	lv_label_set_text(item->lbl_title,name.c_str());
	lv_label_set_long_mode(item->lbl_title,LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
	lv_obj_set_grid_cell(item->lbl_title,LV_GRID_ALIGN_STRETCH,1,2,LV_GRID_ALIGN_CENTER,0,1);

	item->lbl_checked=lv_image_create(item->btn_body);
	lv_obj_set_hidden(item->lbl_checked,true);
	auto mdi=fonts_get("mdi-icon");
	if(mdi)lv_obj_set_style_text_font(item->lbl_checked,mdi,0);
	lv_image_set_src(item->lbl_checked,"\xf3\xb0\x84\xac"); /* mdi-check */
	lv_obj_set_grid_cell(item->lbl_checked,LV_GRID_ALIGN_END,2,1,LV_GRID_ALIGN_CENTER,0,1);

	lv_obj_update_layout(item->btn_body);
	auto h=lv_obj_get_height(item->btn_body);
	auto ch=lv_obj_get_content_height(item->btn_body);
	lv_obj_set_height(item->btn_body,h-ch+s);

	worker_add(std::bind(&ui_draw_choose_image::load_cover,this,item));
}
