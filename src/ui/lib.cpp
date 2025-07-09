#include"ui.h"
#include<map>
#include<functional>
#include"gui.h"

struct event_ctx{
	std::function<void(lv_event_t*)>cb;
};

static void def_event_cb(lv_event_t*ev){
	auto ctx=static_cast<event_ctx*>(lv_event_get_user_data(ev));
	if(ctx&&ctx->cb)ctx->cb(ev);
}

static void obj_event_cleanup(lv_event_t*ev){
	auto obj=static_cast<lv_obj_t*>(lv_event_get_user_data(ev));
	for(uint32_t i=0;i<lv_obj_get_event_count(obj);i++){
		auto dsc=lv_obj_get_event_dsc(obj,i);
		if(dsc->cb!=def_event_cb||!dsc->user_data)continue;
		auto ctx=static_cast<event_ctx*>(dsc->user_data);
		dsc->user_data=nullptr;
		delete ctx;
	}
}

void lv_obj_add_event_func(lv_obj_t*obj,const std::function<void(lv_event_t*)>&event_cb,lv_event_code_t filter){
	auto ctx=new event_ctx{event_cb};
	lv_obj_add_event_cb(obj,def_event_cb,filter,ctx);
	for(uint32_t i=0;i<lv_obj_get_event_count(obj);i++){
		auto dsc=lv_obj_get_event_dsc(obj,i);
		if(dsc->filter!=LV_EVENT_DELETE)continue;
		if(dsc->cb==obj_event_cleanup)return;
	}
	lv_obj_add_event_cb(obj,obj_event_cleanup,LV_EVENT_DELETE,obj);
}

void lv_async_call_func(const std::function<void(void)>&cb){
	struct call_ctx{
		std::function<void(void)>cb;
	};
	auto ctx=new call_ctx{cb};
	lv_async_call([](auto v){
		auto ctx=static_cast<call_ctx*>(v);
		if(ctx->cb)ctx->cb();
		delete ctx;
	},ctx);
}

void lv_thread_call_func(const std::function<void(void)>&cb){
	lv_lock();
	lv_async_call_func(cb);
	lv_unlock();
}

void lv_obj_set_checked(lv_obj_t*obj,bool checked){
	if(checked)lv_obj_add_state(obj,LV_STATE_CHECKED);
	else lv_obj_remove_state(obj,LV_STATE_CHECKED);
}

void lv_obj_set_disabled(lv_obj_t*obj,bool disabled){
	if(disabled)lv_obj_add_state(obj,LV_STATE_DISABLED);
	else lv_obj_remove_state(obj,LV_STATE_DISABLED);
}

void lv_obj_set_hidden(lv_obj_t*obj,bool hidden){
	if(hidden)lv_obj_add_flag(obj,LV_OBJ_FLAG_HIDDEN);
	else lv_obj_remove_flag(obj,LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t*lv_list_add_button_ex(lv_obj_t*list,const void*icon,const char*txt){
	static lv_coord_t grid_cols[]={
		LV_GRID_CONTENT,
		LV_GRID_FR(1),
		LV_GRID_TEMPLATE_LAST
	},grid_rows[]={
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};
	auto obj=lv_obj_class_create_obj(&lv_list_button_class,list);
	lv_obj_class_init_obj(obj);
	lv_obj_set_grid_dsc_array(obj,grid_cols,grid_rows);
	if(icon){
		auto img=lv_image_create(obj);
		auto mdi=fonts_get("mdi-icon");
		if(mdi)lv_obj_set_style_text_font(img,mdi,0);
		lv_image_set_src(img,icon);
		lv_obj_set_grid_cell(img,LV_GRID_ALIGN_START,0,txt?1:2,LV_GRID_ALIGN_CENTER,0,1);
	}
	if(txt){
		auto label=lv_label_create(obj);
		lv_label_set_text(label,txt);
		lv_label_set_long_mode(label,LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
		lv_obj_set_flex_grow(label,1);
		lv_obj_set_grid_cell(label,LV_GRID_ALIGN_STRETCH,icon?1:0,icon?1:2,LV_GRID_ALIGN_CENTER,0,1);
	}
	return obj;
}

void lv_group_add_focus_cb(lv_group_t*group,const std::function<void(lv_group_t*)>&cb){
	static std::map<lv_group_t*,std::vector<std::function<void(lv_group_t*)>>>focus_cb{};
	if(!focus_cb.contains(group)){
		focus_cb[group]={};
		lv_group_set_focus_cb(group,[](auto grp){
			for(auto&cb:focus_cb[grp])cb(grp);
		});
	}
	focus_cb[group].push_back(cb);
}

lv_obj_t*lv_create_mask(const std::function<void()>&cb){
	auto mask=lv_obj_create(lv_screen_active());
	lv_obj_set_size(mask,lv_pct(100),lv_pct(100));
	lv_obj_set_style_bg_color(mask,lv_color_black(),0);
	lv_obj_set_style_bg_opa(mask,LV_OPA_50,0);
	lv_obj_set_style_border_width(mask,0,0);
	lv_obj_set_style_radius(mask,0,0);
	lv_obj_set_style_pad_all(mask,0,0);
	lv_obj_add_flag(mask,LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_func(mask,std::bind(cb),LV_EVENT_CLICKED);
	lv_obj_move_foreground(mask);
	return mask;
}
