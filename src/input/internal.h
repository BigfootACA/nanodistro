#ifndef INPUT_INTERNAL_H
#define INPUT_INTERNAL_H
#include<lvgl.h>
#include<yaml-cpp/yaml.h>

class input_backend{
	public:
		virtual std::vector<lv_indev_t*>init(const YAML::Node&cfg)=0;
};

struct input_backend_create{
	const std::string name;
	std::shared_ptr<input_backend>(*create)();
};
extern const std::vector<input_backend_create>input_backend_creates;

#endif
