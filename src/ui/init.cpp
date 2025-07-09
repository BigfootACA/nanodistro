#include"ui.h"
#include"log.h"
#include"configs.h"
#include<lvgl.h>

lv_obj_t*current_view=nullptr;
lv_obj_t*current_page=nullptr;
std::shared_ptr<ui_draw>current_draw=nullptr;

int gui_init(){
	bool dark=LV_THEME_DEFAULT_DARK;
	if(auto v=config["nanodistro"]["theme"]["dark"])
		dark=v.as<bool>();
	auto disp=display_get_disp();
	disp->theme=lv_theme_default_init(
		disp,
		lv_palette_main(LV_PALETTE_BLUE),
		lv_palette_main(LV_PALETTE_RED),
		dark,gui_def_font
	);
	log_info("initialize with {} theme",dark?"dark":"light");
	auto scr=lv_screen_active();

	current_view=lv_obj_create(scr);
	lv_obj_set_style_bg_opa(current_view,LV_OPA_TRANSP,0);
	lv_obj_set_style_border_width(current_view,0,0);
	lv_obj_set_style_radius(current_view,0,0);
	lv_obj_set_style_pad_all(current_view,0,0);
	lv_obj_set_size(current_view,lv_pct(100),lv_pct(100));
	lv_obj_center(current_view);

	msgbox_init();
	inputbox_init();
	ui_switch_page("hello");
	return 0;
}

void ui_switch_page(const std::string&page){
	for(auto&pg:pages){
		if(pg.name!=page)continue;
		current_draw=pg.create();
		if(!current_draw)continue;
		lv_obj_clean(current_view);

		current_page=lv_obj_create(current_view);
		lv_obj_set_style_bg_opa(current_page,LV_OPA_TRANSP,0);
		lv_obj_set_style_border_width(current_page,0,0);
		lv_obj_set_style_radius(current_page,0,0);
		lv_obj_set_size(current_page,lv_pct(100),lv_pct(100));
		lv_obj_center(current_page);
		current_draw->draw(current_page);
		return;
	}
	log_error("page {} not found",page);
}
