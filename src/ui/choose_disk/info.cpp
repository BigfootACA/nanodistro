#include<variant>
#include<sys/sysmacros.h>
#include"internal.h"
#include"builtin.h"
#include"readable.h"
#include"str-utils.h"
#include"log.h"
#include"configs.h"
#include"gui.h"

void ui_draw_choose_disk::draw_disk_items(){
	lv_group_remove_obj(btn_refresh);
	lv_group_remove_obj(btn_next);
	clean();

	for(auto&disk:data_disks){
		auto gui=std::make_shared<disk_gui_item>();
		gui->item=disk;
		draw_disk_item(gui);
		disks.push_back(gui);
	}

	auto grp=lv_group_get_default();
	lv_group_add_obj(grp,btn_refresh);
	lv_group_add_obj(grp,btn_next);
	set_spinner(false);
	lv_group_focus_obj(btn_refresh);
	if(disks.empty())show_disk_placeholder();

	auto last_devname=installer_context["disk"]["devname"].asString();
	if(!last_devname.empty())for(auto&disk:disks){
		if(disk->item->devname!=last_devname)continue;
		disk_set_selected(disk,true);
		set_selected_disk(disk);
		lv_obj_scroll_to_view(disk->btn_body,LV_ANIM_OFF);
		break;
	}

	setup_hotplug();
}

static builtin_file*choose_icon(const std::shared_ptr<disk_gui_item>&item){
	extern builtin_file file_svg_cdrom;
	extern builtin_file file_svg_disk;
	extern builtin_file file_svg_flash;
	extern builtin_file file_svg_floppy;
	extern builtin_file file_svg_mmc;
	extern builtin_file file_svg_usb;
	if(item->item->info["MEDIA"]=="CDROM")return &file_svg_cdrom;
	if(item->item->info["MEDIA"]=="Floppy")return &file_svg_floppy;
	if(item->item->info["BUS"]=="USB")return &file_svg_usb;
	if(item->item->info["BUS"]=="MMC")return &file_svg_mmc;
	if(item->item->info["BUS"]=="SPI")return &file_svg_flash;
	return &file_svg_disk;
}

void ui_draw_choose_disk::draw_disk_item(const std::shared_ptr<disk_gui_item>&item){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_FR(1),
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	};
	item->btn_body=lv_obj_class_create_obj(&lv_list_button_class,lst_disk);
	lv_obj_class_init_obj(item->btn_body);
	lv_obj_set_grid_dsc_array(item->btn_body,grid_cols,grid_rows);
	auto f1=std::bind(&ui_draw_choose_disk::disk_item_click,this,std::placeholders::_1);
	lv_obj_add_event_func(item->btn_body,f1,LV_EVENT_CLICKED);

	auto font=fonts_get("ui-small");

	auto s=lv_dpx(64);
	auto icon=choose_icon(item);
	item->img_width=s;
	item->img_dsc.header.w=s;
	item->img_dsc.header.h=s;
	item->img_dsc.header.cf=LV_COLOR_FORMAT_ARGB8888;
	item->img_dsc.header.magic=LV_IMAGE_HEADER_MAGIC;
	item->img_dsc.header.stride=s*sizeof(lv_color32_t);
	item->img_dsc.data=(const uint8_t*)icon->data;
	item->img_dsc.data_size=icon->size;
	item->img_cover=lv_image_create(item->btn_body);
	lv_obj_set_size(item->img_cover,s,s);
	lv_image_set_src(item->img_cover,&item->img_dsc);
	lv_obj_set_grid_cell(item->img_cover,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,0,2);

	item->lbl_title=lv_label_create(item->btn_body);
	lv_label_set_text(item->lbl_title,item->item->devname.c_str());
	lv_label_set_long_mode(item->lbl_title,LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
	lv_obj_set_grid_cell(item->lbl_title,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_CENTER,0,1);

	item->lbl_size=lv_label_create(item->btn_body);
	lv_obj_set_style_text_color(item->lbl_size,lv_palette_main(LV_PALETTE_GREY),0);
	lv_label_set_long_mode(item->lbl_size,LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
	lv_obj_set_grid_cell(item->lbl_size,LV_GRID_ALIGN_END,2,1,LV_GRID_ALIGN_CENTER,0,1);
	lv_obj_set_style_text_color(item->lbl_size,lv_palette_main(LV_PALETTE_GREY),0);
	if(font)lv_obj_set_style_text_font(item->lbl_size,font,0);
	lv_label_set_text(item->lbl_size,format_size_float(item->item->info_num["DISKSIZE"]).c_str());

	item->lbl_subtitle=lv_label_create(item->btn_body);
	lv_label_set_long_mode(item->lbl_subtitle,LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
	lv_obj_set_grid_cell(item->lbl_subtitle,LV_GRID_ALIGN_STRETCH,1,2,LV_GRID_ALIGN_CENTER,1,1);
	lv_obj_set_style_text_color(item->lbl_subtitle,lv_palette_main(LV_PALETTE_GREY),0);
	if(font)lv_obj_set_style_text_font(item->lbl_subtitle,font,0);
	lv_label_set_text_fmt(item->lbl_subtitle,"%s %s %s",
		item->item->info["PROTOCOL"].c_str(),
		item->item->info["MEDIA"].c_str(),
		item->item->info["PRODUCT"].c_str()
	);

	item->lbl_checked=lv_image_create(item->btn_body);
	lv_obj_set_hidden(item->lbl_checked,true);
	auto mdi=fonts_get("mdi-icon");
	if(mdi)lv_obj_set_style_text_font(item->lbl_checked,mdi,0);
	lv_image_set_src(item->lbl_checked,"\xf3\xb0\x84\xac"); /* mdi-check */
	lv_obj_set_grid_cell(item->lbl_checked,LV_GRID_ALIGN_END,3,1,LV_GRID_ALIGN_CENTER,0,2);

	lv_obj_update_layout(item->btn_body);
	auto h=lv_obj_get_height(item->btn_body);
	auto ch=lv_obj_get_content_height(item->btn_body);
	lv_obj_set_height(item->btn_body,h-ch+s);
}

void ui_draw_choose_disk::show_info(const std::shared_ptr<disk_gui_item>&item){
	std::string buffer;
	struct info_helper{
		std::string title;
	};
	struct info_num_helper{
		std::string title;
		const char**units=nullptr;
		size_t block=1024;
	};
	auto font=lv_obj_get_style_text_font(lbl_info,0);
	auto swidth=lv_text_get_width(" ",1,font,0);
	auto add_info=[&](const std::string&title,const std::string&content){
		std::string cont=content;
		str_trim(cont);
		if(cont.empty())return;
		buffer+=title;
		buffer+=": ";
		auto width=lv_text_get_width(title.c_str(),title.length(),font,0);
		auto qwidth=width/swidth;
		if(qwidth<20)buffer+=std::string(20-qwidth,' ');
		buffer+=content;
		buffer+='\n';
	};
	auto add_info_num=[&](const info_num_helper&i,const std::string&title,const uint64_t val){
		std::string sval;
		if(!i.units)sval=std::to_string(val);
		else sval=format_size_float(val,i.units,i.block);
		add_info(title,sval);
	};
	const std::map<std::string,std::variant<info_helper,info_num_helper>>info_print_map={
		{"DISKSIZE",         info_num_helper{_("Disk Size"),            size_units_ib}},
		{"SECTOR_COUNT",     info_num_helper{_("Sector Count"),         nullptr}},
		{"SECTOR_SIZE",      info_num_helper{_("Sector Size"),          size_units_ib}},
		{"DEVPATH",          info_helper{_("Device path")}},
		{"SUBSYSTEM",        info_helper{_("Subsystem")}},
		{"BUS",              info_helper{_("Bus")}},
		{"PROTOCOL",         info_helper{_("Protocol")}},
		{"MEDIA",            info_helper{_("Media")}},
		{"AREA",             info_helper{_("Area")}},
		{"VENDOR",           info_helper{_("Vendor")}},
		{"MODEL",            info_helper{_("Model")}},
		{"SERIAL",           info_helper{_("Serial Number")}},
		{"WWID",             info_helper{_("WWID")}},
		{"USB_PORT",         info_helper{_("USB Port")}},
		{"USB_SPEED",        info_helper{_("USB Speed")}},
		{"USB_SPEED",        info_num_helper{_("USB Speed"),            size_units_ibs}},
		{"USB_VERSION",      info_helper{_("USB Version")}},
		{"USB_VENDOR_ID",    info_helper{_("USB Vendor ID")}},
		{"USB_PRODUCT_ID",   info_helper{_("USB Product ID")}},
		{"USB_SERIAL",       info_helper{_("USB Serial Number")}},
		{"USB_MANUFACTURER", info_helper{_("USB Manufacturer")}},
		{"USB_PRODUCT",      info_helper{_("USB Product")}},
		{"SCSI_HOST",        info_helper{_("SCSI Host")}},
		{"SCSI_CHANNEL",     info_helper{_("SCSI Channel")}},
		{"SCSI_ID",          info_helper{_("SCSI ID")}},
		{"SCSI_LUN",         info_helper{_("SCSI LUN")}},
		{"PCIE_SLOT",        info_helper{_("PCIe slot")}},
		{"PCIE_LANE_WIDTH",  info_helper{_("PCIe lanes")}},
		{"PCIE_LANE_BAND",   info_num_helper{_("PCIe lane baudwidth"),  size_units_ts, 1000}},
		{"PCIE_LANE_SPEED",  info_num_helper{_("PCIe lane speed"),      size_units_ibs}},
		{"PCIE_TOTAL_BAND",  info_num_helper{_("PCIe total baudwidth"), size_units_ts, 1000}},
		{"PCIE_TOTAL_SPEED", info_num_helper{_("PCIe total speed"),     size_units_ibs}},
		{"PCIE_GEN",         info_helper{_("PCIe version")}},
	};
	add_info(_("Device name"),item->item->devname);
	add_info(_("Device node"),std::format("{}:{}",major(item->item->dev),minor(item->item->dev)));
	add_info(_("Device node path"),item->item->path_dev);
	add_info(_("Sysfs path"),item->item->path_sysfs);
	for(auto&[k,m]:info_print_map){
		if(auto i=std::get_if<info_helper>(&m)){
			if(!item->item->info.contains(k))continue;
			add_info(i->title,item->item->info[k]);
		}else if(auto i=std::get_if<info_num_helper>(&m)){
			if(!item->item->info_num.contains(k))continue;
			add_info_num(*i,i->title,item->item->info_num[k]);
		}
	}
	lv_label_set_text(lbl_info,buffer.c_str());
}

Json::Value disk_item::to_json()const{
	Json::Value j{};
	j["devname"]=devname;
	j["sysfs"]=path_sysfs;
	j["device"]=path_dev;
	j["devnode"]=std::format("{}:{}",major(dev),minor(dev));
	for(auto&[k,v]:info)j["info"][k]=v;
	for(auto&[k,v]:info_num)j["info"][k]=v;
	for(auto&[k,v]:uevent)j["uevent"][k]=v;
	for(auto&[k,v]:dev_uevent)j["dev_uevent"][k]=v;
	return j;
}
