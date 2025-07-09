#if defined(USE_LIB_SDL2)
#include"internal.h"
#include"error.h"

class display_instance_sdl2:public display_instance{
};

class display_backend_sdl2:public display_backend{
	public:
		std::shared_ptr<display_instance>init(const YAML::Node&cfg)override;
};

std::shared_ptr<display_backend>display_backend_create_sdl2(){
	return std::make_shared<display_backend_sdl2>();
}

std::shared_ptr<display_instance>display_backend_sdl2::init(const YAML::Node&cfg){
	int width=960,height=540;
	if(auto v=cfg["width"])
		width=v.as<int>();
	if(auto v=cfg["height"])
		height=v.as<int>();
	auto disp=lv_sdl_window_create(width,height);
	if(!disp)throw RuntimeError("failed to create SDL2 display");
	if(auto v=cfg["dpi"])
		disp->dpi=v.as<int>();
	auto ins=std::make_shared<display_instance_sdl2>();
	ins->disp=disp;
	return ins;
}
#endif
