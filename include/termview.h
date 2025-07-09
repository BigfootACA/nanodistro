#ifndef LV_TERMVIEW_H
#define LV_TERMVIEW_H
#include<string>
#include<functional>
#include<lvgl.h>
#include"libtsm.h"
using termview_write_cb=std::function<void(lv_obj_t*tv,const char *u8,size_t len)>;
using termview_osc_cb=std::function<void(lv_obj_t*tv,const char *u8,size_t len)>;
using termview_resize_cb=std::function<void(lv_obj_t*tv,uint32_t cols,uint32_t rows)>;
typedef struct{
	lv_canvas_t canvas;
	lv_coord_t glyph_height;
	lv_coord_t glyph_width;
	lv_coord_t width;
	lv_coord_t height;
	lv_coord_t drag_y_last;
	uint32_t cols;
	uint32_t rows;
	uint32_t mods;
	uint32_t max_sb;
	bool cust_font;
	tsm_age_t age;
	size_t mem_size;
	lv_color32_t*buffer;
	tsm_screen*screen;
	tsm_vte*vte;
	const lv_font_t*font_reg;
	const lv_font_t*font_bold;
	const lv_font_t*font_ital;
	const lv_font_t*font_bold_ital;
	termview_osc_cb osc_cb;
	termview_write_cb write_cb;
	termview_resize_cb resize_cb;
	uint32_t last_update=0;
}lv_termview_t;

extern void lv_termview_set_mods(lv_obj_t*tv,uint32_t mods);
extern void lv_termview_set_max_scroll_buffer_size(lv_obj_t*tv,uint32_t max_sb);
extern void lv_termview_set_osc_cb(lv_obj_t*tv,termview_osc_cb cb);
extern void lv_termview_set_write_cb(lv_obj_t*tv,termview_write_cb cb);
extern void lv_termview_set_resize_cb(lv_obj_t*tv,termview_resize_cb cb);
extern void lv_termview_update(lv_obj_t*tv);
extern void lv_termview_set_font(lv_obj_t*tv,const lv_font_t*font);
extern void lv_termview_set_font_regular(lv_obj_t*tv,lv_font_t*font);
extern void lv_termview_set_font_bold(lv_obj_t*tv,lv_font_t*font);
extern void lv_termview_set_font_italic(lv_obj_t*tv,lv_font_t*font);
extern void lv_termview_set_font_bold_italic(lv_obj_t*tv,lv_font_t*font);
extern void lv_termview_resize(lv_obj_t*tv);
extern uint32_t lv_termview_get_cols(lv_obj_t*tv);
extern uint32_t lv_termview_get_rows(lv_obj_t*tv);
extern void lv_termview_input(lv_obj_t*tv,const char*str);
extern void lv_termview_line_print(lv_obj_t*tv,const char*str);
extern void lv_termview_line_prints(lv_obj_t*tv,const std::string&str);
extern tsm_screen*lv_termview_get_screen(lv_obj_t*tv);
extern tsm_vte*lv_termview_get_vte(lv_obj_t*tv);
extern lv_obj_t*lv_termview_create(lv_obj_t*par);
#endif
