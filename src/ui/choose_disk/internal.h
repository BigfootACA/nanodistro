#ifndef UI_CHOOSE_DISK_INTERNAL_H
#define UI_CHOOSE_DISK_INTERNAL_H
#include"../ui.h"
#include<lvgl.h>
#include<map>
#include<json/json.h>
#include<sys/stat.h>

struct disk_item{
	std::string devname{};
	std::string path_sysfs{};
	std::string path_dev{};
	std::map<std::string,std::string>uevent{};
	std::map<std::string,std::string>dev_uevent{};
	std::map<std::string,std::string>info{};
	std::map<std::string,uint64_t>info_num{};
	dev_t dev;
	Json::Value to_json()const;
};

struct disk_gui_item{
	std::shared_ptr<disk_item>item=nullptr;
	lv_obj_t*btn_body=nullptr;
	lv_obj_t*img_cover=nullptr;
	lv_obj_t*lbl_title=nullptr;
	lv_obj_t*lbl_subtitle=nullptr;
	lv_obj_t*lbl_size=nullptr;
	lv_obj_t*lbl_checked=nullptr;
	lv_image_dsc_t img_dsc{};
	lv_coord_t img_width=0;
	bool selected=false;
};

class ui_draw_choose_disk:public ui_draw{
	public:
		~ui_draw_choose_disk();
		void draw(lv_obj_t*cont)override;
		void draw_buttons(lv_obj_t*cont);
		void draw_spinner(lv_obj_t*cont);
		void set_spinner(bool show);
		void load_disks();
		void setup_hotplug();
		std::shared_ptr<disk_item>load_disk(const std::string&dev);
		void disk_item_click(lv_event_t*ev);
		void draw_disk_item(const std::shared_ptr<disk_gui_item>&item);
		void draw_disk_items();
		void disk_set_selected(const std::shared_ptr<disk_gui_item>&item,bool selected);
		void set_selected_disk(const std::shared_ptr<disk_gui_item>&item);
		void show_info(const std::shared_ptr<disk_gui_item>&item);
		void btn_refresh_cb(lv_event_t*);
		void btn_next_cb(lv_event_t*);
		void btn_back_cb(lv_event_t*);
		bool on_hotplug(uint64_t id,std::map<std::string,std::string>&uevent);
		void hotplug_add(const std::string&dev);
		void hotplug_remove(const std::string&dev);
		void show_info_placeholder();
		void show_disk_placeholder();
		void clean();
		std::mutex lock{};
		uint64_t uevent_id=0;
		lv_obj_t*btn_refresh=nullptr;
		lv_obj_t*btn_back=nullptr;
		lv_obj_t*btn_next=nullptr;
		lv_obj_t*lst_disk=nullptr;
		lv_obj_t*box_info=nullptr;
		lv_obj_t*lbl_loading=nullptr;
		lv_obj_t*lbl_info=nullptr;
		lv_obj_t*lbl_info_placeholder=nullptr;
		lv_obj_t*lbl_disk_placeholder=nullptr;
		struct{
			lv_obj_t*lbl_info_title=nullptr;
			lv_obj_t*lbl_info_content=nullptr;
		}info[2];
		uint8_t info_sel=0;
		lv_obj_t*box_loading=nullptr;
		lv_obj_t*spinner=nullptr;
		lv_obj_t*mask=nullptr;
		std::list<std::shared_ptr<disk_gui_item>>disks{};
		std::list<std::shared_ptr<disk_item>>data_disks{};
		std::shared_ptr<disk_gui_item>selected_disk=nullptr;
};

#endif
