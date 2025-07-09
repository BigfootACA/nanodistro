#include"internal.h"
#include"configs.h"
#include"error.h"
#include"log.h"
#include"gui.h"

std::shared_ptr<display_instance>cur_display=nullptr;

int display_init(){
	std::string backend;
	auto dcfg=config["nanodistro"]["display"];
	if(!dcfg)throw InvalidArgument("no display configuration found");
	if(!dcfg["backend"])throw InvalidArgument("no display backend set");
	backend=dcfg["backend"].as<std::string>();
	auto&cs=display_backend_creates;
	auto it=std::find_if(
		cs.begin(),cs.end(),
		[&](auto dc){return dc.name==backend;}
	);
	if(it==cs.end())throw RuntimeError("display backend {} not found",backend);
	auto be=it->create();
	if(!be)throw RuntimeError("failed to create display backend {}",backend);
	cur_display=be->init(dcfg);
	if(!cur_display)throw RuntimeError("failed to initialize display backend {}",backend);
	log_info("initialized display with backend {}",backend);
	auto disp=cur_display->disp;
	if(!disp)throw RuntimeError("failed to create display");
	log_info("screen {}x{} dpi {}",disp->hor_res,disp->ver_res,disp->dpi);
	return 0;
}

void display_instance::force_flush(){
	if(disp)lv_async_call_func([]{
		lv_obj_invalidate(lv_screen_active());
	});
}

lv_display_t*display_get_disp(const std::shared_ptr<display_instance>&d){
	if(!d)throw RuntimeError("display instance is null");
	if(!d->disp)throw RuntimeError("display is null");
	return d->disp;
}
