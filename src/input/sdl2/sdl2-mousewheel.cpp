#include"../internal.h"
#include"error.h"

class input_backend_sdl2_mousewheel:public input_backend{
	public:
		std::vector<lv_indev_t*>init(const YAML::Node&cfg)override;
};

std::shared_ptr<input_backend>input_backend_create_sdl2_mousewheel(){
	return std::make_shared<input_backend_sdl2_mousewheel>();
}

std::vector<lv_indev_t*>input_backend_sdl2_mousewheel::init(const YAML::Node&cfg){
	auto indev=lv_sdl_mousewheel_create();
	if(!indev)throw RuntimeError("failed to create SDL2 mousewheel");
	return {indev};
}
