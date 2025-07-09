#include"log.h"
#include"network.h"
#include"gui.h"

void ui_network_panel_hello::draw(lv_obj_t*obj){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};
	ui_network_panel::draw(obj);
	lv_obj_set_size(panel,lv_pct(100),lv_pct(100));

	auto box=lv_obj_create(panel);
	lv_obj_set_size(box,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_obj_set_style_radius(box,0,0);
	lv_obj_set_style_pad_all(box,0,0);
	lv_obj_set_style_bg_opa(box,0,0);
	lv_obj_set_style_border_width(box,0,0);
	lv_obj_set_grid_dsc_array(box,grid_cols,grid_rows);
	lv_obj_align(box,LV_ALIGN_CENTER,0,-14);

	auto mdi=fonts_get("mdi-large");
	if(mdi){
		auto img=lv_image_create(box);
		lv_obj_set_style_text_font(img,mdi,0);
		lv_image_set_src(img,"\xf3\xb0\x96\x9f"); /* mdi-web */
		lv_obj_set_grid_cell(img,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,0,1);
	}

	auto text=lv_label_create(box);
	lv_label_set_text(text,_(
		"The distro flash wizard need download some files from internet\n"
		"You need to connect to a network to continue\n"
		"Please connect to a network from the list in left"
	));
	lv_obj_set_style_text_align(text,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_grid_cell(text,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_CENTER,mdi?1:0,mdi?1:2);
}
