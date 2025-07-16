#include"internal.h"
#include"builtin.h"
#include"log.h"
#include"worker.h"

std::shared_ptr<ui_draw>ui_create_mass_storage_running(){
	return std::make_shared<ui_draw_mass_storage_running>();
}

void ui_draw_mass_storage_running::draw(lv_obj_t*cont){
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
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	};
	if(!mass_storage_config_default.initialized)
		throw std::runtime_error("mass storage config not initialized");

	lv_obj_set_grid_dsc_array(cont,grid_cols,grid_rows);

	spinner=lv_spinner_create(cont);
	lv_obj_set_grid_cell(spinner,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,1,1);

	extern builtin_file file_svg_usb;
	auto s=lv_dpx(128);
	cover_dsc.header.w=s;
	cover_dsc.header.h=s;
	cover_dsc.header.cf=LV_COLOR_FORMAT_ARGB8888;
	cover_dsc.header.magic=LV_IMAGE_HEADER_MAGIC;
	cover_dsc.header.stride=s*sizeof(lv_color32_t);
	cover_dsc.data=(const uint8_t*)file_svg_usb.data;
	cover_dsc.data_size=file_svg_usb.size;
	img_cover=lv_image_create(cont);
	lv_obj_set_grid_cell(img_cover,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_obj_set_size(img_cover,s,s);
	lv_image_set_src(img_cover,&cover_dsc);

	lbl_status=lv_label_create(cont);
	lv_obj_set_style_pad_all(lbl_status,lv_dpx(24),0);
	lv_obj_set_grid_cell(lbl_status,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,2,1);

	btn_stop=lv_button_create(cont);
	auto lbl_btn_stop=lv_label_create(btn_stop);
	auto f1=std::bind(&ui_draw_mass_storage_running::btn_stop_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_stop,f1,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_stop,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_CENTER,3,1);
	lv_label_set_text(lbl_btn_stop,_("Stop"));

	set_status(false,_("Starting..."));
	worker_add(std::bind(&ui_draw_mass_storage_running::do_start,this));
}

void ui_draw_mass_storage_running::btn_stop_cb(lv_event_t*){
	set_status(false,_("Stopping..."));
	worker_add(std::bind(&ui_draw_mass_storage_running::do_stop,this));
}

void ui_draw_mass_storage_running::set_status(bool ready,const std::string&status){
	lv_obj_set_hidden(spinner,ready);
	lv_obj_set_hidden(img_cover,!ready);
	lv_obj_set_disabled(btn_stop,!ready);
	lv_label_set_text(lbl_status,status.c_str());
}

void ui_draw_mass_storage_running::do_start(){
	try{
		mass_storage_config_default.apply();
	}catch(std::exception&exc){
		log_exception(exc,"failed to start usb mass storage");
		std::string reason=exc.what();
		lv_async_call_func([reason]{
			ui_switch_page("mass-storage");
			msgbox_show(std::string(_("Failed to start USB mass storage"))+"\n"+reason);
		});
		return;
	}
	log_info("usb mass storage started");
	lv_async_call_func([this]{
		set_status(true,_("USB Mass Storage running"));
	});
}

void ui_draw_mass_storage_running::do_stop(){
	try{
		mass_storage_config_default.clean();
	}catch(...){}
	log_info("usb mass storage stopped");
	lv_async_call_func([]{
		ui_switch_page("mass-storage");
	});
}
