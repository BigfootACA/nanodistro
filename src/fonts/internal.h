#ifndef FONTS_INTERNAL_H
#define FONTS_INTERNAL_H
#include<lvgl.h>
#include<yaml-cpp/yaml.h>

class font_backend;
struct font_desc;

class font_handler{
	public:
		virtual ~font_handler();
		std::shared_ptr<font_desc>desc=nullptr;
};

struct font_desc{
	font_desc(const YAML::Node&cfg,const std::shared_ptr<font_backend>&backend);
	std::string name{};
	std::string path{};
	uint32_t size=0;
	YAML::Node cfg{};
	lv_font_t*font=nullptr;
	std::shared_ptr<font_backend>backend=nullptr;
	std::shared_ptr<font_handler>handler=nullptr;
};

class font_backend{
	public:
		virtual void load_from_config(const std::shared_ptr<font_desc>&desc);
		virtual void load_by_name(const std::shared_ptr<font_desc>&desc);
		virtual void load_by_path(const std::shared_ptr<font_desc>&desc){}
		virtual std::vector<std::string>get_font_filename(const std::string&name)=0;
};

struct font_backend_create{
	const std::string name;
	std::shared_ptr<font_backend>(*create)();
};
extern const std::vector<font_backend_create>font_backend_creates;

#endif
