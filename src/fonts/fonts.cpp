#include"fs-utils.h"
#include"path-utils.h"
#include"internal.h"
#include"configs.h"
#include"error.h"
#include"gui.h"
#include"log.h"

static std::map<std::string,std::vector<std::shared_ptr<font_desc>>>fonts{};
static std::map<std::string,std::shared_ptr<font_backend>>font_backends{};
extern "C"{
	lv_font_t*gui_def_font=(lv_font_t*)&lv_font_montserrat_14;
}

font_desc::font_desc(
	const YAML::Node&cfg,
	const std::shared_ptr<font_backend>&backend
):cfg(cfg),backend(backend){
}

font_handler::~font_handler(){
	if(desc&&desc->font==gui_def_font)
		gui_def_font=(lv_font_t*)&lv_font_montserrat_14;
}

lv_font_t*fonts_get(const std::string&name){
	auto it=fonts.find(name);
	if(it==fonts.end())return nullptr;
	for(auto&font:it->second)
		if(font->font)
			return font->font;
	return nullptr;
}

std::vector<std::string>fonts_get_folders(){
	std::vector<std::string>folders{};
	auto fcfg=config["nanodistro"]["fonts-dir"];
	if(!fcfg||!fcfg.IsSequence())return {};
	for(auto dir:fcfg)folders.push_back(dir.as<std::string>());
	return folders;
}

void font_backend::load_by_name(const std::shared_ptr<font_desc>&desc){
	if(desc->name.empty())throw InvalidArgument("no font name specified");
	auto filenames=get_font_filename(desc->name);
	for(auto&dir:fonts_get_folders())for(auto&filename:filenames){
		auto path=path_join(dir,filename);
		if(!fs_exists(path))continue;
		desc->path=path;
		load_by_path(desc);
		if(!desc->font)throw RuntimeError(
			"failed to load font {}",path
		);
		return;
	}
	log_warning("font {} not found",desc->name);
}

void font_backend::load_from_config(const std::shared_ptr<font_desc>&desc){
	desc->size=14;
	if(auto v=desc->cfg["size"])
		desc->size=v.as<size_t>();
	if(auto v=desc->cfg["size-mode"]){
		auto mode=v.as<std::string>();
		if(mode=="dpx")desc->size=lv_dpx(desc->size);
		else if(mode!="fixed")throw InvalidArgument(
			"unknown size mode {}",mode
		);
	}
	if(auto v=desc->cfg["path"]){
		desc->path=v.as<std::string>();
		load_by_path(desc);
		return;
	}
	if(auto v=desc->cfg["name"]){
		desc->name=v.as<std::string>();
		load_by_name(desc);
		return;
	}
	throw InvalidArgument("no font sepcified");
}

static std::shared_ptr<font_backend>fonts_get_backend(const std::string&name){
	auto it=font_backends.find(name);
	if(it!=font_backends.end())return it->second;
	auto&cs=font_backend_creates;
	auto it2=std::find_if(
		cs.begin(),cs.end(),
		[&](auto dc){return dc.name==name;}
	);
	if(it2==cs.end())return nullptr;
	auto backend=it2->create();
	if(!backend)return nullptr;
	font_backends[name]=backend;
	return backend;
}

int fonts_init(){
	auto fcfg=config["nanodistro"]["fonts"];
	if(!fcfg||!fcfg.IsMap())
		throw InvalidArgument("no fonts configuration found");
	for(auto fm:fcfg){
		auto id=fm.first.as<std::string>();
		if(!fm.second.IsSequence())
			throw InvalidArgument("invalid font config {}",id);
		for(auto font:fm.second){
			if(!font["backend"])
				throw InvalidArgument("no font backend set");
			auto backend=font["backend"].as<std::string>();
			if(fonts.contains(id))throw InvalidArgument(
				"font {} already exists",id
			);
			auto fb=fonts_get_backend(backend);
			if(!fb)throw RuntimeError(
				"font backend {} not found",backend
			);
			auto fd=std::make_shared<font_desc>(font,fb);
			fb->load_from_config(fd);
			if(!fd->font){
				log_warning("failed to load font {} with backend {}",fd->path,backend);
				continue;
			}
			if(!fonts.contains(id))fonts[id]={};
			fonts[id].push_back(fd);
			log_info(
				"create font {} size {} with {} from backend {}",
				id,fd->size,fd->path,backend
			);
		}
	}
	for(auto&[id,fv]:fonts){
		lv_font_t*last=nullptr;
		for(auto it=fv.rbegin();it!=fv.rend();it++){
			if(!(*it)->font)continue;
			(*it)->font->fallback=last;
			last=(*it)->font;
		}
	}

	gui_def_font=fonts_get("ui-normal");
	if(!gui_def_font)throw InvalidArgument("default font not found");

	auto mdi=fonts_get("mdi-normal");
	if(mdi)gui_def_font->fallback=mdi;
	return 0;
}
