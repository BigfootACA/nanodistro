#include"../internal.h"
#include"error.h"

class font_backend_freetype2:public font_backend{
	public:
		void load_by_path(const std::shared_ptr<font_desc>&desc)override;
		inline std::vector<std::string>get_font_filename(const std::string&name)override{return {name+".ttf"};}
};

class font_handler_freetype2:public font_handler{
	public:
		~font_handler_freetype2()override;
};

font_handler_freetype2::~font_handler_freetype2(){
	font_handler::~font_handler();
	if(desc&&desc->font){
		lv_freetype_font_delete(desc->font);
		desc->font=nullptr;
	}
}

std::shared_ptr<font_backend>font_backend_create_freetype2(){
	return std::make_shared<font_backend_freetype2>();
}

void font_backend_freetype2::load_by_path(const std::shared_ptr<font_desc>&desc){
	auto render=LV_FREETYPE_FONT_RENDER_MODE_BITMAP;
	int style=LV_FREETYPE_FONT_STYLE_NORMAL;
	if(auto v=desc->cfg["render-mode"]){
		auto mode=v.as<std::string>();
		if(mode=="bitmap")render=LV_FREETYPE_FONT_RENDER_MODE_BITMAP;
		else if(mode=="outline")render=LV_FREETYPE_FONT_RENDER_MODE_OUTLINE;
		else throw InvalidArgument("unknown render mode {}",mode);
	}
	if(auto v=desc->cfg["italic"];v&&v.as<bool>())
		style|=LV_FREETYPE_FONT_STYLE_ITALIC;
	if(auto v=desc->cfg["bold"];v&&v.as<bool>())
		style|=LV_FREETYPE_FONT_STYLE_BOLD;
	if(desc->path.empty())throw InvalidArgument("no font path specified");
	if(desc->size==0)throw InvalidArgument("no font size specified");
	desc->handler=std::make_shared<font_handler_freetype2>();
	desc->handler->desc=desc;
	desc->font=lv_freetype_font_create(
		desc->path.c_str(),render,desc->size,
		(lv_freetype_font_style_t)style
	);
}
