#include"internal.h"
#include"configs.h"
#include"std-utils.h"
#include"error.h"
#include"log.h"

std::list<lv_indev_t*>indevs{};
static std::map<std::string,std::shared_ptr<input_backend>>input_backends{};

static std::shared_ptr<input_backend>input_get_backend(const std::string&name){
	auto it=input_backends.find(name);
	if(it!=input_backends.end())return it->second;
	auto&cs=input_backend_creates;
	auto it2=std::find_if(
		cs.begin(),cs.end(),
		[&](auto dc){return dc.name==name;}
	);
	if(it2==cs.end())return nullptr;
	auto backend=it2->create();
	if(!backend)return nullptr;
	input_backends[name]=backend;
	return backend;
}

int input_init(){
	auto icfg=config["nanodistro"]["input"];
	if(!icfg||!icfg.IsSequence())
		throw InvalidArgument("no input configuration found");
	auto grp=lv_group_get_default();
	if(!grp){
		grp=lv_group_create();
		lv_group_set_default(grp);
	}
	for(auto input:icfg){
		if(!input["backend"])
			throw InvalidArgument("no input backend set");
		auto backend=input["backend"].as<std::string>();
		auto fb=input_get_backend(backend);
		if(!fb)throw RuntimeError("input backend {} not found",backend);
		auto indev=fb->init(input);
		if(indev.empty())throw InvalidArgument("failed to create input with backend {}",backend);
		log_info("create input device from backend {}",backend);
		for(auto&dev:indev)lv_indev_set_group(dev,grp);
		indevs.insert(indevs.end(),indev.begin(),indev.end());
	}
	if(indevs.empty())
		throw InvalidArgument("no input found");
	return 0;
}
