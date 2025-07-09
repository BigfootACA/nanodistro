#include"../internal.h"
#include"error.h"

class input_backend_sdl2_mouse:public input_backend{
	public:
		std::vector<lv_indev_t*>init(const YAML::Node&cfg)override;
};

std::shared_ptr<input_backend>input_backend_create_sdl2_mouse(){
	return std::make_shared<input_backend_sdl2_mouse>();
}

std::vector<lv_indev_t*>input_backend_sdl2_mouse::init(const YAML::Node&cfg){
	auto indev=lv_sdl_mouse_create();
	if(!indev)throw RuntimeError("failed to create SDL2 mouse");
	return {indev};
}
