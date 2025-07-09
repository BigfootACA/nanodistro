#ifndef GUI_H
#define GUI_H
#include<lvgl.h>
#include<list>
#include<atomic>
#include<vector>
#include<string>
#include<memory>
#include<functional>
class display_instance;
extern std::shared_ptr<display_instance>cur_display;
extern std::list<lv_indev_t*>indevs;
extern std::atomic<bool>gui_running;
extern std::atomic<bool>gui_pause;
extern int gui_init();
extern int display_init();
extern int input_init();
extern int image_init();
extern int fonts_init();
extern void display_switch_tty(int tty);
extern void display_switch_gui();
extern void display_handle_signal(int sig);
extern lv_display_t*display_get_disp(const std::shared_ptr<display_instance>&d=cur_display);
extern void lv_obj_add_event_func(lv_obj_t*obj,const std::function<void(lv_event_t*)>&event_cb,lv_event_code_t filter);
extern void lv_async_call_func(const std::function<void(void)>&cb);
extern void lv_thread_call_func(const std::function<void(void)>&cb);
extern void lv_obj_set_checked(lv_obj_t*obj,bool checked);
extern void lv_obj_set_disabled(lv_obj_t*obj,bool disabled);
extern void lv_obj_set_hidden(lv_obj_t*obj,bool hidden);
extern std::vector<std::string>fonts_get_folders();
extern lv_font_t*fonts_get(const std::string&name);
#endif
