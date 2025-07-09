#ifndef DISPLAY_INTERNAL_H
#define DISPLAY_INTERNAL_H
#include<lvgl.h>
#include<memory>
#include<yaml-cpp/yaml.h>

class display_instance{
	public:
		virtual ~display_instance()=default;
		virtual void fill_color(lv_color_t){}
		virtual void force_flush();
		lv_display_t*disp=nullptr;
};

class display_backend{
	public:
		virtual std::shared_ptr<display_instance>init(const YAML::Node&cfg)=0;
};

struct display_backend_create{
	const std::string name;
	std::shared_ptr<display_backend>(*create)();
};
extern const std::vector<display_backend_create>display_backend_creates;

extern void set_console(int con);
extern void bind_vtconsole(int value);
extern void display_setup_console(int tty=8);
extern void display_deinit_console();
#endif
