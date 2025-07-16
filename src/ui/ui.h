#ifndef UI_H
#define UI_H
#include<any>
#include<memory>
#include<string>
#include<lvgl.h>
#include<functional>
#include"gui.h"
class ui_draw{
	public:
		virtual ~ui_draw()=default;
		virtual void draw(lv_obj_t*cont){}
		virtual void draw(lv_obj_t*cont,std::any data){draw(cont);}
};
struct ui_page{
	const std::string name;
	std::function<std::shared_ptr<ui_draw>()>create;
};
extern const std::vector<ui_page>pages;
extern lv_obj_t*current_view;
extern lv_obj_t*current_page;
extern std::shared_ptr<ui_draw>current_draw;
extern void ui_switch_page(const std::string&page,std::any data={});
extern void lv_textarea_set_keyboard(lv_obj_t*obj);
extern void lv_group_add_focus_cb(lv_group_t*group,const std::function<void(lv_group_t*)>&cb);
extern lv_obj_t*lv_create_mask(const std::function<void()>&cb=[]{});
extern lv_obj_t*lv_list_add_button_ex(lv_obj_t*list,const void*icon,const char*txt);
extern void msgbox_init();
extern void msgbox_show(const std::string&msg);
extern void inputbox_set_keyboard_show(bool show);
extern void inputbox_init();
extern void inputbox_show(const std::string&msg);
extern void inputbox_show_textarea(const std::string&msg,lv_obj_t*ta);
extern void inputbox_bind_textarea(const std::string&msg,lv_obj_t*ta);
#endif
