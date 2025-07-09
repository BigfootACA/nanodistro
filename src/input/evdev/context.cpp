#include<dirent.h>
#include"internal.h"
#include"uevent.h"
#include"fs-utils.h"
#include"path-utils.h"
#include"cleanup.h"
#include"error.h"
#include"gui.h"

std::shared_ptr<evdev_context>evdev_ctx=nullptr;

std::shared_ptr<input_backend>input_backend_create_evdev(){
	return std::make_shared<input_backend_evdev>();
}

std::vector<lv_indev_t*>input_backend_evdev::init(const YAML::Node&cfg){
	if(evdev_ctx)throw RuntimeError("only one evdev allowed");
	evdev_ctx=std::make_shared<evdev_context>();
	evdev_ctx->init();
	evdev_ctx->init_indev();
	evdev_ctx->apply(cfg);
	evdev_ctx->probe_all();
	return {
		evdev_ctx->encoder.indev,
		evdev_ctx->pointer.indev,
		evdev_ctx->keypad.indev,
	};
}

void evdev_context::apply(const YAML::Node&cfg){

}

std::vector<evdev_device*>evdev_context::probe_all(){
	std::vector<evdev_device*>ret{};
	std::string dir="/dev/input";
	if(!fs_exists(dir))return {};
	auto d=opendir(dir.c_str());
	if(!d)return {};
	cleanup_func cleanup(std::bind(closedir,d));
	while(auto e=readdir(d))try{
		std::string name=e->d_name;
		if(e->d_type!=DT_CHR)continue;
		if(!name.starts_with("event"))continue;
		start(path_join(dir,e->d_name));
	}catch(std::exception&exc){
		log_exception(exc,"failed to probe evdev device");
	}
	return ret;
}

void evdev_context::init(){
	auto f1=std::bind(&evdev_context::process_uevent,this,std::placeholders::_1,std::placeholders::_2);
	uevent_id=uevent_listener::add(f1);
}

evdev_device*evdev_context::start(const std::string&dev){
	if(dev.empty())return nullptr;
	auto device=new evdev_device(this);
	device->path=dev;
	device->init_device();
	devices.push_back(device);
	return device;
}

void evdev_context::stop(const std::string&dev){
	for(auto&d:devices){
		if(d->path!=dev)continue;
		d->stop();
		break;
	}
}

bool evdev_context::process_uevent(uint64_t id,std::map<std::string,std::string>&uevent){
	if(id!=uevent_id)return true;
	if(uevent.empty()||!uevent.contains("SUBSYSTEM"))return true;
	if(uevent["SUBSYSTEM"]!="input")return true;
	if(!uevent.contains("DEVNAME"))return true;
	auto action=uevent["ACTION"];
	auto devname=uevent["DEVNAME"];
	if(!devname.starts_with("input/event"))return true;
	auto fdev=path_join("/dev",devname);
	try{
		log_info("receive uevent {} event for {}",action,fdev);
		if(action=="add")start(fdev);
		if(action=="remove")stop(fdev);
	}catch(ErrnoErrorImpl&exc){
		log_exception(exc,"skip device {}",fdev);
	}catch(std::exception&exc){
		log_exception(exc,"failed to process uevent for {}",fdev);
	}
	return true;
}


void evdev_context::handle_tty(int tty){
	log_info("request switch to tty {}",tty);
	display_switch_tty(tty);
}
