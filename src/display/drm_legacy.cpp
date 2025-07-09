#if defined(USE_LIB_LIBDRM)
#include"internal.h"
#include"cleanup.h"
#include"error.h"

class display_backend_drm:public display_backend{
	public:
		lv_display_t*init(const YAML::Node&cfg)override;
};

std::shared_ptr<display_backend>display_backend_create_drm(){
	return std::make_shared<display_backend_drm>();
}

lv_display_t*display_backend_drm::init(const YAML::Node&cfg){
	std::string dev{};
	int64_t connector=-1;
	auto disp=lv_linux_drm_create();
	if(!disp)throw RuntimeError("failed to create drm display");
	cleanup_func cleanup(std::bind(lv_display_delete,disp));
	if(auto v=cfg["dev"])dev=v.as<std::string>();
	if(auto v=cfg["connector"])connector=v.as<int64_t>();
	if(dev.empty())throw RuntimeError("drm device not set");
	lv_linux_drm_set_file(disp,dev.c_str(),connector);
	if(auto v=cfg["dpi"])lv_display_set_dpi(disp,v.as<int>());
	cleanup.kill();
	return disp;
}

#endif
