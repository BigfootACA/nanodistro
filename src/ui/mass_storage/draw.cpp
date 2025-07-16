#include"internal.h"
#include"log.h"
#include "src/misc/lv_event.h"
#include"std-utils.h"
#include"str-utils.h"
#include"../context.h"

std::shared_ptr<ui_draw>ui_create_mass_storage(){
	return std::make_shared<ui_draw_mass_storage>();
}

void ui_draw_mass_storage::draw_disks_buttons(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
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
	lv_obj_set_grid_cell(box,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,2,1);

	btn_reset=lv_button_create(box);
	auto lbl_btn_reset=lv_label_create(btn_reset);
	auto f1=std::bind(&ui_draw_mass_storage::btn_reset_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_reset,f1,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_reset,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_reset,_("Reset"));

	btn_remove=lv_button_create(box);
	auto lbl_btn_remove=lv_label_create(btn_remove);
	auto f2=std::bind(&ui_draw_mass_storage::btn_remove_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_remove,f2,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_remove,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_END,0,1);
	lv_obj_set_disabled(btn_remove,true);
	lv_label_set_text(lbl_btn_remove,_("Remove"));

	btn_clear=lv_button_create(box);
	auto lbl_btn_clear=lv_label_create(btn_clear);
	auto f3=std::bind(&ui_draw_mass_storage::btn_clear_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_clear,f3,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_clear,LV_GRID_ALIGN_CENTER,2,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_clear,_("Clear"));

	btn_add=lv_button_create(box);
	auto lbl_btn_add=lv_label_create(btn_add);
	auto f4=std::bind(&ui_draw_mass_storage::btn_add_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_add,f4,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_add,LV_GRID_ALIGN_END,3,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_add,_("Add"));
}

void ui_draw_mass_storage::draw_options(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
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

	lbl_udc=lv_label_create(box);
	lv_obj_set_grid_cell(lbl_udc,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_label_set_text(lbl_udc,_("USB Controller"));

	drop_udc=lv_dropdown_create(box);
	auto f1=std::bind(&ui_draw_mass_storage::drop_udc_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(drop_udc,f1,LV_EVENT_VALUE_CHANGED);
	lv_obj_set_grid_cell(drop_udc,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_CENTER,0,1);

	btn_udc_reload=lv_button_create(box);
	auto lbl_btn_udc_reload=lv_label_create(btn_udc_reload);
	auto f2=std::bind(&ui_draw_mass_storage::btn_udc_reload_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_udc_reload,f2,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_udc_reload,LV_GRID_ALIGN_STRETCH,2,1,LV_GRID_ALIGN_STRETCH,0,1);
	auto mdi=fonts_get("mdi-icon");
	if(mdi)lv_obj_set_style_text_font(lbl_btn_udc_reload,mdi,0);
	lv_label_set_text(lbl_btn_udc_reload,"\xf3\xb0\x91\x90"); /* mdi-refresh */

	lbl_stor_mode=lv_label_create(box);
	lv_obj_set_grid_cell(lbl_stor_mode,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,1,1);
	lv_label_set_text(lbl_stor_mode,_("Storage Mode"));

	drop_stor_mode=lv_dropdown_create(box);
	auto f3=std::bind(&ui_draw_mass_storage::drop_stor_mode_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(drop_stor_mode,f3,LV_EVENT_VALUE_CHANGED);
	lv_obj_set_grid_cell(drop_stor_mode,LV_GRID_ALIGN_STRETCH,1,2,LV_GRID_ALIGN_CENTER,1,1);
	lv_dropdown_clear_options(drop_stor_mode);
	lv_dropdown_add_option(drop_stor_mode,_("USB Attached SCSI (UAS)"),STORAGE_MODE_UAS);
	lv_dropdown_add_option(drop_stor_mode,_("USB Mass Storage (UMS)"),STORAGE_MODE_MASS);

	check_multi_lun=lv_checkbox_create(box);
	lv_checkbox_set_text(check_multi_lun,_("Use multiple LUN instead of multiple function"));
	lv_obj_set_grid_cell(check_multi_lun,LV_GRID_ALIGN_STRETCH,0,3,LV_GRID_ALIGN_CENTER,2,1);

	check_scsi_direct=lv_checkbox_create(box);
	lv_checkbox_set_text(check_scsi_direct,_("Use SCSI direct access"));
	lv_obj_set_grid_cell(check_scsi_direct,LV_GRID_ALIGN_STRETCH,0,3,LV_GRID_ALIGN_CENTER,3,1);

	check_readonly=lv_checkbox_create(box);
	lv_checkbox_set_text(check_readonly,_("Read-only mode"));
	lv_obj_set_grid_cell(check_readonly,LV_GRID_ALIGN_STRETCH,0,3,LV_GRID_ALIGN_CENTER,4,1);

	if(mass_storage_config_default.initialized){
		lv_dropdown_set_selected(drop_stor_mode,mass_storage_config_default.mode,LV_ANIM_OFF);
		lv_obj_set_checked(check_multi_lun,mass_storage_config_default.multi_lun);
		lv_obj_set_checked(check_scsi_direct,mass_storage_config_default.scsi_direct);
		lv_obj_set_checked(check_readonly,mass_storage_config_default.readonly);
	}
}

void ui_draw_mass_storage::draw_opts_buttons(lv_obj_t*cont){
	static lv_coord_t grid_cols[]={
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
	auto f1=std::bind(&ui_draw_mass_storage::btn_back_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_back,f1,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_back,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_END,0,1);
	lv_label_set_text(lbl_btn_back,_("Back"));

	btn_start=lv_button_create(box);
	auto lbl_btn_start=lv_label_create(btn_start);
	auto f2=std::bind(&ui_draw_mass_storage::btn_start_cb,this,std::placeholders::_1);
	lv_obj_add_event_func(btn_start,f2,LV_EVENT_CLICKED);
	lv_obj_set_grid_cell(btn_start,LV_GRID_ALIGN_END,2,1,LV_GRID_ALIGN_END,0,1);
	lv_obj_set_disabled(btn_start,true);
	lv_label_set_text(lbl_btn_start,_("Start"));
}

void ui_draw_mass_storage::drop_udc_cb(lv_event_t*){
	update();
}

void ui_draw_mass_storage::drop_stor_mode_cb(lv_event_t*){
	update();
}

void ui_draw_mass_storage::btn_reset_cb(lv_event_t*){
	mass_storage_config_default.reset_disks();
	load_disks();
	update();
}

void ui_draw_mass_storage::btn_remove_cb(lv_event_t*){
	if(!selected_disk)return;
	auto cur=selected_disk;
	set_selected_disk(nullptr);
	lv_obj_delete(cur->btn_body);
	mass_storage_config_default.disks.remove(cur->devname);
	disks.remove(cur);
	if(disks.empty()){
		lv_obj_clean(lst_disk);
		lbl_disk_placeholder=nullptr;
		show_disk_placeholder();
	}
	update();
}

void ui_draw_mass_storage::btn_clear_cb(lv_event_t*){
	set_selected_disk(nullptr);
	lv_obj_clean(lst_disk);
	lbl_disk_placeholder=nullptr;
	disks.clear();
	mass_storage_config_default.disks.clear();
	show_disk_placeholder();
	update();
}

void ui_draw_mass_storage::btn_add_cb(lv_event_t*){
	auto ctx=std::make_shared<disk_context>();
	ctx->title=_("Select disk to mount");
	ctx->back_page="mass-storage";
	ctx->next_page="mass-storage";
	ctx->callback=[](auto disk){
		auto devname=disk["devname"].asString();
		if(!mass_storage_config_default.initialized)return;
		if(std_contains(mass_storage_config_default.disks,devname))return;
		if(!disk_can_exclusive(devname)){
			msgbox_show(ssprintf(_("Disk %s is busy, please unmount it first"),devname.c_str()));
			return;
		}
		mass_storage_config_default.disks.push_back(devname);
	};
	update();
	ui_switch_page("choose-disk",ctx);
}

void ui_draw_mass_storage::btn_start_cb(lv_event_t*){
	try{
		if(disks.empty())
			throw std::runtime_error(_("No disks to mount"));
		if(lv_dropdown_get_selected(drop_udc)<=0)
			throw std::runtime_error(_("Please select USB controller"));
		auto stor_mode=(storage_mode)lv_dropdown_get_selected(drop_stor_mode);
		switch(stor_mode){
			case STORAGE_MODE_UAS:
				if(lv_obj_has_state(check_readonly,LV_STATE_CHECKED))
					throw std::runtime_error(_("USB Attached SCSI does not support read-only mode"));
				if(!lv_obj_has_state(check_multi_lun,LV_STATE_CHECKED))
					throw std::runtime_error(_("USB Attached SCSI must use multiple LUN mode"));
			break;
			case STORAGE_MODE_MASS:
				if(lv_obj_has_state(check_scsi_direct,LV_STATE_CHECKED))
					throw std::runtime_error(_("USB Mass Storage does not support SCSI direct mode"));
			break;
			default:throw std::runtime_error(_("Unknown storage mode"));
		}
		char buff[128]{};
		lv_dropdown_get_selected_str(drop_udc,buff,sizeof(buff)-1);
		mass_storage_config_default.udc=buff;
		mass_storage_config_default.mode=stor_mode;
		mass_storage_config_default.multi_lun=lv_obj_has_state(check_multi_lun,LV_STATE_CHECKED);
		mass_storage_config_default.scsi_direct=lv_obj_has_state(check_scsi_direct,LV_STATE_CHECKED);
		mass_storage_config_default.readonly=lv_obj_has_state(check_readonly,LV_STATE_CHECKED);
		ui_switch_page("mass-running");
	}catch(std::exception&exc){
		msgbox_show(exc.what());
	}
}

void ui_draw_mass_storage::btn_back_cb(lv_event_t*){
	ui_switch_page("hello");
}

void ui_draw_mass_storage::btn_udc_reload_cb(lv_event_t*){
	load_udc();
	update();
}

void ui_draw_mass_storage::draw(lv_obj_t*cont){
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
	lv_label_set_text(lbl_title,_("Mount options"));
	lv_obj_set_grid_cell(lbl_title,LV_GRID_ALIGN_CENTER,0,2,LV_GRID_ALIGN_CENTER,0,1);

	lst_disk=lv_list_create(cont);
	lv_obj_set_grid_cell(lst_disk,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,1,1);

	box_opts=lv_obj_create(cont);
	lv_obj_set_grid_cell(box_opts,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,1,1);

	try{
		mass_storage_config_default.initialize();
		mass_storage_config_default.clean();
	}catch(...){}
	draw_disks_buttons(cont);
	draw_opts_buttons(cont);
	draw_options(box_opts);
	load_udc();
	load_disks();
	update();
}

void ui_draw_mass_storage::clean(){
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

void ui_draw_mass_storage::update(){
	lv_obj_set_disabled(btn_start,disks.empty()||lv_dropdown_get_selected(drop_udc)<=0);
	switch((storage_mode)lv_dropdown_get_selected(drop_stor_mode)){
		case STORAGE_MODE_UAS:
			lv_obj_set_disabled(check_multi_lun,true);
			lv_obj_set_disabled(check_scsi_direct,false);
			lv_obj_set_disabled(check_readonly,true);
			lv_obj_set_checked(check_multi_lun,true);
			lv_obj_set_checked(check_readonly,false);
		break;
		case STORAGE_MODE_MASS:
			lv_obj_set_disabled(check_multi_lun,false);
			lv_obj_set_disabled(check_scsi_direct,true);
			lv_obj_set_disabled(check_readonly,false);
			lv_obj_set_checked(check_scsi_direct,false);
		break;
		default:;
	}
}

void ui_draw_mass_storage::disk_set_selected(const std::shared_ptr<mass_disk_item>&item,bool s){
	lv_obj_set_grid_cell(item->lbl_size,LV_GRID_ALIGN_END,2,1+(!s),LV_GRID_ALIGN_CENTER,0,1);
	lv_obj_set_hidden(item->lbl_checked,!s);
	item->selected=s;
}

void ui_draw_mass_storage::set_selected_disk(const std::shared_ptr<mass_disk_item>&item){
	selected_disk=item;
	lv_obj_set_disabled(btn_remove,!item);
}

void ui_draw_mass_storage::disk_item_click(lv_event_t*ev){
	auto tgt=lv_event_get_target(ev);
	set_selected_disk(nullptr);
	for(auto&disk:disks){
		if(disk->btn_body==tgt){
			disk_set_selected(disk,!disk->selected);
			if(disk->selected)set_selected_disk(disk);
		}else disk_set_selected(disk,false);
	}
}

void ui_draw_mass_storage::show_disk_placeholder(){
	if(lbl_disk_placeholder){
		lv_obj_set_hidden(lbl_disk_placeholder,false);
	}else{
		lbl_disk_placeholder=lv_label_create(lst_disk);
		lv_obj_set_style_text_align(lbl_disk_placeholder,LV_TEXT_ALIGN_CENTER,0);
		lv_obj_set_size(lbl_disk_placeholder,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
		lv_label_set_text(lbl_disk_placeholder,_("No any disk added"));
		lv_obj_center(lbl_disk_placeholder);
	}
	lv_obj_set_style_layout(lst_disk,LV_LAYOUT_NONE,0);
}
