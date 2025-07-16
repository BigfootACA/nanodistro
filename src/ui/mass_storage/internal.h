#ifndef UI_MASS_STORAGE_INTERNAL_H
#define UI_MASS_STORAGE_INTERNAL_H
#include"../ui.h"
#include "src/lv_api_map_v8.h"
#include<lvgl.h>
#include<map>
#include<json/json.h>
#include<sys/stat.h>

enum storage_mode{
	STORAGE_MODE_UAS,
	STORAGE_MODE_MASS,
};

struct mass_disk_item{
	std::string devname{};
	lv_obj_t*btn_body=nullptr;
	lv_obj_t*img_cover=nullptr;
	lv_obj_t*lbl_title=nullptr;
	lv_obj_t*lbl_size=nullptr;
	lv_obj_t*lbl_checked=nullptr;
	bool selected=false;
};

struct mass_storage_config{
	std::list<std::string>disks{};
	std::string udc{};
	std::string str_serial_num="1234567890";
	storage_mode mode=STORAGE_MODE_UAS;
	bool multi_lun=false;
	bool scsi_direct=false;
	bool readonly=false;
	bool initialized=false;
	void initialize();
	void reset_disks();
	void apply();
	void clean();
};

class ui_draw_mass_storage:public ui_draw{
	public:
		void draw(lv_obj_t*cont)override;
		void draw_disks_buttons(lv_obj_t*cont);
		void draw_opts_buttons(lv_obj_t*cont);
		void draw_options(lv_obj_t*cont);
		void disk_item_click(lv_event_t*ev);
		void draw_disk_items();
		void disk_set_selected(const std::shared_ptr<mass_disk_item>&item,bool selected);
		void set_selected_disk(const std::shared_ptr<mass_disk_item>&item);
		void btn_reset_cb(lv_event_t*);
		void btn_remove_cb(lv_event_t*);
		void btn_clear_cb(lv_event_t*);
		void btn_add_cb(lv_event_t*);
		void btn_back_cb(lv_event_t*);
		void btn_start_cb(lv_event_t*);
		void btn_udc_reload_cb(lv_event_t*);
		void drop_udc_cb(lv_event_t*);
		void drop_stor_mode_cb(lv_event_t*);
		void load_udc();
		void load_disks();
		void show_disk_placeholder();
		void clean();
		void update();
		std::mutex lock{};
		lv_obj_t*btn_reset=nullptr;
		lv_obj_t*btn_remove=nullptr;
		lv_obj_t*btn_clear=nullptr;
		lv_obj_t*btn_add=nullptr;
		lv_obj_t*btn_back=nullptr;
		lv_obj_t*btn_start=nullptr;
		lv_obj_t*lst_disk=nullptr;
		lv_obj_t*box_opts=nullptr;
		lv_obj_t*lbl_disk_placeholder=nullptr;
		lv_obj_t*lbl_udc=nullptr;
		lv_obj_t*drop_udc=nullptr;
		lv_obj_t*btn_udc_reload=nullptr;
		lv_obj_t*lbl_stor_mode=nullptr;
		lv_obj_t*drop_stor_mode=nullptr;
		lv_obj_t*check_multi_lun=nullptr;
		lv_obj_t*check_scsi_direct=nullptr;
		lv_obj_t*check_readonly=nullptr;
		uint8_t info_sel=0;
		std::list<std::shared_ptr<mass_disk_item>>disks{};
		std::shared_ptr<mass_disk_item>selected_disk=nullptr;
};

class ui_draw_mass_storage_running:public ui_draw{
	public:
		void draw(lv_obj_t*cont)override;
		void btn_stop_cb(lv_event_t*);
		void set_status(bool ready,const std::string&status);
		void do_start();
		void do_stop();
		lv_obj_t*img_cover=nullptr;
		lv_obj_t*spinner=nullptr;
		lv_obj_t*lbl_status=nullptr;
		lv_obj_t*btn_stop=nullptr;
		lv_img_dsc_t cover_dsc{};
};

extern mass_storage_config mass_storage_config_default;
extern std::list<std::string>mass_storage_list_disks();
extern std::list<std::string>mass_storage_list_udc();
extern void gadget_clean(const std::string&func);
extern void gadget_clean_all();
extern bool disk_can_exclusive(const std::string&devname);
#endif
