#include<fcntl.h>
#include<dirent.h>
#include<linux/fs.h>
#include"internal.h"
#include"readable.h"
#include"path-utils.h"
#include"str-utils.h"
#include"fs-utils.h"
#include"builtin.h"
#include"error.h"
#include"cleanup.h"
#include"log.h"

mass_storage_config mass_storage_config_default{};

void mass_storage_config::reset_disks(){
	disks.clear();
	auto ds=mass_storage_list_disks();
	for(auto&d:ds)if(disk_can_exclusive(d))
		disks.push_back(d);
}

void mass_storage_config::initialize(){
	if(initialized)return;
	reset_disks();
	auto udcs=mass_storage_list_udc();
	bool filter=true;
	while(udc.empty()){
		for(auto&u:udcs){
			if(filter){
				if(str_contains(u,"dummy"))continue;
				if(str_contains(u,"usbip"))continue;
				if(str_contains(u,"test"))continue;
			}
			udc=u;
			break;
		}
		if(!filter)break;
		filter=false;
	}
	mode=STORAGE_MODE_UAS;
	multi_lun=true;
	scsi_direct=false;
	readonly=false;
	initialized=true;
}

bool disk_can_exclusive(const std::string&devname){
	auto path=path_join("/dev",devname);
	int fd=open(path.c_str(),O_RDWR|O_NDELAY|O_EXCL|O_CLOEXEC);
	if(fd<0)return false;
	close(fd);
	return true;
}

std::list<std::string>mass_storage_list_disks(){
	std::list<std::string>ret{};
	std::string dir="/sys/class/block";
	if(!fs_exists(dir))return {};
	fs_list_dir(dir,[&](const std::string&name,int dt){
		if(dt!=DT_LNK||name[0]=='.')return true;
		auto path=path_join(dir,name);
		if(fs_exists(path_join(path,"partition")))return true;
		auto size=fs_read_all(path_join(path,"size"));
		if(size.empty()||str_trim_to("size")=="0")return true;
		if(name.starts_with("md"))return true;
		if(name.starts_with("dm-"))return true;
		if(name.starts_with("loop"))return true;
		if(name.starts_with("ram"))return true;
		if(name.starts_with("zram"))return true;
		if(name.starts_with("nbd"))return true;
		if(name.starts_with("mtdblock"))return true;
		if(name.starts_with("mmcblk")&&str_contains(name,"rpmb"))return true;
		if(name.starts_with("mmcblk")&&str_contains(name,"boot"))return true;
		ret.push_back(name);
		return true;
	});
	return ret;
}

std::list<std::string>mass_storage_list_udc(){
	std::list<std::string>ret{};
	std::string dir="/sys/class/udc";
	if(!fs_exists(dir))return {};
	fs_list_dir(dir,[&](const std::string&name,int dt){
		if(dt!=DT_LNK||name[0]=='.')return true;
		auto path=path_join(dir,name);
		if(!fs_exists(path_join(path,"current_speed")))return true;
		ret.push_back(name);
		return true;
	});
	return ret;
}

void ui_draw_mass_storage::load_udc(){
	std::string old_selected{};
	if(lv_dropdown_get_selected(drop_udc)!=0){
		char buff[128]{};
		lv_dropdown_get_selected_str(drop_udc,buff,sizeof(buff)-1);
		if(buff[0])old_selected=buff;
	}
	if(old_selected.empty()&&mass_storage_config_default.initialized)
		old_selected=mass_storage_config_default.udc;
	lv_dropdown_clear_options(drop_udc);
	lv_dropdown_add_option(drop_udc,_("(None)"),0);
	auto udc=mass_storage_list_udc();
	if(udc.empty())log_warning("no any UDC found");
	uint32_t new_selected=0;
	int i=0;
	for(auto&u:udc){
		i++;
		lv_dropdown_add_option(drop_udc,u.c_str(),i);
		if(!old_selected.empty()&&u==old_selected)
			new_selected=i;

	}
	if(!udc.empty()&&new_selected==0)new_selected=1;
	lv_dropdown_set_selected(drop_udc,new_selected,LV_ANIM_OFF);
}

void ui_draw_mass_storage::load_disks(){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	};
	static lv_image_dsc_t img_dsc{};
	extern builtin_file file_svg_disk;
	const auto s=lv_dpx(32);
	img_dsc.header.w=s;
	img_dsc.header.h=s;
	img_dsc.header.cf=LV_COLOR_FORMAT_ARGB8888;
	img_dsc.header.magic=LV_IMAGE_HEADER_MAGIC;
	img_dsc.header.stride=s*sizeof(lv_color32_t);
	img_dsc.data=(const uint8_t*)file_svg_disk.data;
	img_dsc.data_size=file_svg_disk.size;
	lv_obj_clean(lst_disk);
	disks.clear();
	set_selected_disk(nullptr);
	if(!mass_storage_config_default.initialized)
		mass_storage_config_default.initialize();
	auto&ds=mass_storage_config_default.disks;
	if(ds.empty()){
		lbl_disk_placeholder=nullptr;
		show_disk_placeholder();
		return;
	}
	lv_obj_set_style_layout(lst_disk,LV_LAYOUT_FLEX,0);
	lv_list_add_text(lst_disk,_("Devices to mount"));
	for(auto&dev:ds){
		auto item=std::make_shared<mass_disk_item>();
		item->devname=dev;

		item->btn_body=lv_obj_class_create_obj(&lv_list_button_class,lst_disk);
		lv_obj_class_init_obj(item->btn_body);
		lv_obj_set_grid_dsc_array(item->btn_body,grid_cols,grid_rows);
		auto f1=std::bind(&ui_draw_mass_storage::disk_item_click,this,std::placeholders::_1);
		lv_obj_add_event_func(item->btn_body,f1,LV_EVENT_CLICKED);

		auto font=fonts_get("ui-small");

		item->img_cover=lv_image_create(item->btn_body);
		lv_obj_set_size(item->img_cover,s,s);
		lv_image_set_src(item->img_cover,&img_dsc);
		lv_obj_set_grid_cell(item->img_cover,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_CENTER,0,1);

		item->lbl_title=lv_label_create(item->btn_body);
		lv_label_set_text(item->lbl_title,dev.c_str());
		lv_label_set_long_mode(item->lbl_title,LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
		lv_obj_set_grid_cell(item->lbl_title,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_CENTER,0,1);

		item->lbl_size=lv_label_create(item->btn_body);
		lv_obj_set_style_text_color(item->lbl_size,lv_palette_main(LV_PALETTE_GREY),0);
		lv_label_set_long_mode(item->lbl_size,LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
		lv_obj_set_grid_cell(item->lbl_size,LV_GRID_ALIGN_END,2,1,LV_GRID_ALIGN_CENTER,0,1);
		lv_obj_set_style_text_color(item->lbl_size,lv_palette_main(LV_PALETTE_GREY),0);
		if(font)lv_obj_set_style_text_font(item->lbl_size,font,0);
		lv_label_set_text(item->lbl_size,"");
		auto path=path_join("/dev",dev);
		if(fs_exists(path))try{
			size_t size=0;
			auto fd=open(path.c_str(),O_RDONLY|O_CLOEXEC);
			if(fd<0)throw ErrnoError("failed to open {}",path);
			cleanup_func closer(std::bind(&close,fd));
			xioctl(fd,BLKGETSIZE64,&size);
			lv_label_set_text(item->lbl_size,format_size_float(size).c_str());
		}catch(std::exception&exc){
			log_exception(exc,"failed to get disk size for {}",dev);
		}

		item->lbl_checked=lv_image_create(item->btn_body);
		lv_obj_set_hidden(item->lbl_checked,true);
		auto mdi=fonts_get("mdi-icon");
		if(mdi)lv_obj_set_style_text_font(item->lbl_checked,mdi,0);
		lv_image_set_src(item->lbl_checked,"\xf3\xb0\x84\xac"); /* mdi-check */
		lv_obj_set_grid_cell(item->lbl_checked,LV_GRID_ALIGN_END,3,1,LV_GRID_ALIGN_CENTER,0,1);

		lv_obj_update_layout(item->btn_body);
		auto h=lv_obj_get_height(item->btn_body);
		auto ch=lv_obj_get_content_height(item->btn_body);
		lv_obj_set_height(item->btn_body,h-ch+s);
		disks.push_back(item);
	}
}
