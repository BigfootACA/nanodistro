#include"internal.h"
#include"uevent.h"
#include"log.h"
#include"error.h"

bool ui_draw_choose_disk::on_hotplug(uint64_t id,std::map<std::string,std::string>&uevent){
	if(id!=uevent_id)return true;
	if(uevent.empty()||!uevent.contains("SUBSYSTEM"))return true;
	if(uevent["SUBSYSTEM"]!="block")return true;
	if(!uevent.contains("DEVNAME"))return true;
	auto action=uevent["ACTION"];
	auto devname=uevent["DEVNAME"];
	try{
		log_info("receive uevent {} event for {}",action,devname);
		if(action=="add")hotplug_add(devname);
		if(action=="remove")hotplug_remove(devname);
	}catch(ErrnoErrorImpl&exc){
		log_exception(exc,"skip device {}",devname);
	}catch(std::exception&exc){
		log_exception(exc,"failed to process uevent for {}",devname);
	}
	return true;
}

void ui_draw_choose_disk::hotplug_add(const std::string&dev){
	std::shared_ptr<disk_item>disk;
	{
		std::lock_guard<std::mutex>lk(lock);
		disk=load_disk(dev);
		if(!disk)return;
	}
	lv_lock();
	auto gui=std::make_shared<disk_gui_item>();
	gui->item=disk;
	draw_disk_item(gui);
	disks.push_back(gui);
	if(lbl_disk_placeholder)
		lv_obj_set_hidden(lbl_disk_placeholder,false);
	lv_unlock();
}

void ui_draw_choose_disk::hotplug_remove(const std::string&dev){
	std::shared_ptr<disk_gui_item>gui_disk;
	lv_lock();
	auto f=[&](const auto&item){return item->item->devname==dev;};
	auto it=std::find_if(disks.begin(),disks.end(),f);
	if(it!=disks.end()){
		auto gui_disk=*it;
		disks.erase(it);
		if(selected_disk==gui_disk)
			set_selected_disk(nullptr);
		if(gui_disk->btn_body){
			auto grp=lv_group_get_default();
			if(lv_group_get_focused(grp)==gui_disk->btn_body)
				lv_group_focus_obj(btn_refresh);
			lv_obj_delete(gui_disk->btn_body);
		}
		if(disks.empty())show_disk_placeholder();
	}
	lv_unlock();
	if(gui_disk){
		std::lock_guard<std::mutex>lk(lock);
		data_disks.remove(gui_disk->item);
	}
}

void ui_draw_choose_disk::setup_hotplug(){
	if(uevent_id>0)return;
	auto f=std::bind(
		&ui_draw_choose_disk::on_hotplug,this,
		std::placeholders::_1,std::placeholders::_2
	);
	uevent_id=uevent_listener::add(f);
}

ui_draw_choose_disk::~ui_draw_choose_disk(){
	if(uevent_id>0){
		uevent_listener::remove(uevent_id);
		uevent_id=0;
	}
}
