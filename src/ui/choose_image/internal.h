#ifndef UI_CHOOSE_IMAGE_INTERNAL_H
#define UI_CHOOSE_IMAGE_INTERNAL_H
#include"../ui.h"
#include"url.h"
#include<lvgl.h>
#include<json/json.h>

class data_item{
	public:
		std::string id{};
		lv_obj_t*btn_body=nullptr;
		lv_obj_t*img_cover=nullptr;
		lv_obj_t*lbl_title=nullptr;
		lv_obj_t*lbl_checked=nullptr;
		std::string img_data{};
		lv_image_dsc_t img_dsc{};
		lv_coord_t img_width=0;
		bool selected=false;
		Json::Value data{};
};

class group_item:public data_item{
};

class image_item:public data_item{
	public:
		lv_obj_t*lbl_subtitle=nullptr;
};

class ui_draw_choose_image:public ui_draw{
	public:
		void draw(lv_obj_t*cont)override;
		void draw_buttons(lv_obj_t*cont);
		void draw_spinner(lv_obj_t*cont);
		void set_spinner(bool show);
		void load_images();
		void load_cover(std::shared_ptr<data_item>item);
		void populate_images();
		void populate_image(const std::shared_ptr<image_item>&item);
		void populate_group(const std::shared_ptr<group_item>&item);
		void group_item_click(lv_event_t*ev);
		void image_item_click(lv_event_t*ev);
		void group_set_selected(const std::shared_ptr<group_item>&item,bool selected);
		void image_set_selected(const std::shared_ptr<image_item>&item,bool selected);
		void set_selected_image(const std::shared_ptr<image_item>&item);
		void btn_refresh_cb(lv_event_t*);
		void btn_next_cb(lv_event_t*);
		void btn_back_cb(lv_event_t*);
		void clean();
		lv_obj_t*btn_refresh=nullptr;
		lv_obj_t*btn_next=nullptr;
		lv_obj_t*btn_back=nullptr;
		lv_obj_t*lst_group=nullptr;
		lv_obj_t*lst_image=nullptr;
		lv_obj_t*lbl_group_placeholder=nullptr;
		lv_obj_t*lbl_image_placeholder=nullptr;
		lv_obj_t*lbl_loading=nullptr;
		lv_obj_t*box_loading=nullptr;
		lv_obj_t*spinner=nullptr;
		lv_obj_t*mask=nullptr;
		Json::Value data_groups{};
		Json::Value data_images{};
		url images_url{};
		std::list<std::shared_ptr<group_item>>groups{};
		std::list<std::shared_ptr<image_item>>images{};
		std::shared_ptr<image_item>selected_image=nullptr;
};
#endif
