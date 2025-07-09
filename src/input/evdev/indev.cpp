#include"internal.h"
#include"error.h"
#include"gui.h"

void evdev_indev::add(const keypad_report&report){
	if(gui_pause)return;
	if(lv_indev_get_type(indev)!=LV_INDEV_TYPE_KEYPAD)
		throw RuntimeError("device is not keypad");
	std::lock_guard<std::mutex>lock(mutex);
	queue.push({.k=report});
}

void evdev_indev::add(const pointer_report&report){
	if(gui_pause)return;
	if(lv_indev_get_type(indev)!=LV_INDEV_TYPE_POINTER)
		throw RuntimeError("device is not pointer");
	std::lock_guard<std::mutex>lock(mutex);
	queue.push({.p=report});
}

void evdev_indev::add(const encoder_report&report){
	if(gui_pause)return;
	if(lv_indev_get_type(indev)!=LV_INDEV_TYPE_ENCODER)
		throw RuntimeError("device is not encoder");
	std::lock_guard<std::mutex>lock(mutex);
	queue.push({.e=report});
}

static void evdev_read(lv_indev_t*indev,lv_indev_data_t*data){
	auto ctx=(evdev_context*)lv_indev_get_driver_data(indev);
	if(!ctx)return;
	switch(lv_indev_get_type(indev)){
		case LV_INDEV_TYPE_ENCODER:ctx->encoder_read(indev,data);break;
		case LV_INDEV_TYPE_POINTER:ctx->pointer_read(indev,data);break;
		case LV_INDEV_TYPE_KEYPAD:ctx->keypad_read(indev,data);break;
		default:;
	}
}

void evdev_context::init_indev(){
	encoder.indev=lv_indev_create();
	pointer.indev=lv_indev_create();
	keypad.indev=lv_indev_create();
	if(!encoder.indev||!pointer.indev||!keypad.indev)
		throw RuntimeError("failed to create indev");
	lv_indev_set_type(encoder.indev,LV_INDEV_TYPE_ENCODER);
	lv_indev_set_type(pointer.indev,LV_INDEV_TYPE_POINTER);
	lv_indev_set_type(keypad.indev,LV_INDEV_TYPE_KEYPAD);
	lv_indev_set_mode(encoder.indev,LV_INDEV_MODE_EVENT);
	lv_indev_set_mode(pointer.indev,LV_INDEV_MODE_EVENT);
	lv_indev_set_mode(keypad.indev,LV_INDEV_MODE_EVENT);
	lv_indev_set_read_cb(encoder.indev,evdev_read);
	lv_indev_set_read_cb(pointer.indev,evdev_read);
	lv_indev_set_read_cb(keypad.indev,evdev_read);
	lv_indev_set_driver_data(encoder.indev,this);
	lv_indev_set_driver_data(pointer.indev,this);
	lv_indev_set_driver_data(keypad.indev,this);
}

void evdev_context::pointer_read(lv_indev_t*indev,lv_indev_data_t*data){
	std::lock_guard<std::mutex>lock(pointer.mutex);
	if(!pointer.queue.empty()){
		data->continue_reading=true;
		pointer.last=pointer.queue.front();
		pointer.queue.pop();
	}else data->continue_reading=false;
	data->state=pointer.last.p.press?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
	data->point.x=pointer.last.p.x;
	data->point.y=pointer.last.p.y;
	log_debug(
		"pointer report x {} y {} press {}",
		pointer.last.p.x,pointer.last.p.y,pointer.last.p.press
	);
	draw_mouse();
}

void evdev_context::keypad_read(lv_indev_t*indev,lv_indev_data_t*data){
	std::lock_guard<std::mutex>lock(keypad.mutex);
	if(!keypad.queue.empty()){
		data->continue_reading=true;
		keypad.last=keypad.queue.front();
		keypad.queue.pop();
	}else data->continue_reading=false;
	data->state=keypad.last.k.press?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
	data->key=keypad.last.k.key;
	log_debug(
		"keypad report key {} press {}",
		keypad.last.k.key,keypad.last.k.press
	);
	draw_mouse();
}

void evdev_context::encoder_read(lv_indev_t*indev,lv_indev_data_t*data){
	std::lock_guard<std::mutex>lock(encoder.mutex);
	if(!encoder.queue.empty()){
		data->continue_reading=false;
		encoder.last=encoder.queue.front();
		encoder.queue.pop();
	}else data->continue_reading=false;
	data->enc_diff=encoder.last.e.movement;
	data->continue_reading=!encoder.queue.empty();
	log_debug(
		"encoder report movement {}",
		encoder.last.e.movement
	);
	draw_mouse();
}

void evdev_context::draw_mouse(){
	if(!detect_mouse&&!mouse)return;
	if(!mouse){
		auto mdi=fonts_get("mdi-icon");
		mouse=lv_label_create(lv_screen_active());
		lv_label_set_text(mouse,"\xf3\xb0\x87\x80"); /* mdi-cursor-default */
		lv_obj_set_pos(mouse,pointer_val.x,pointer_val.y);
		lv_indev_set_cursor(pointer.indev,mouse);
		auto dark=lv_theme_default_get()->flags&1;
		auto color=dark?lv_palette_lighten(LV_PALETTE_GREY,4):lv_color_black();
		lv_obj_set_style_text_color(mouse,color,0);
		if(mdi)lv_obj_set_style_text_font(mouse,mdi,0);
	}
	if(!detect_mouse)lv_obj_add_flag(mouse,LV_OBJ_FLAG_HIDDEN);
	else lv_obj_remove_flag(mouse,LV_OBJ_FLAG_HIDDEN);
}

void evdev_context::trigger_read(){
	auto do_read=[](void*d){
		auto indev=(evdev_indev*)d;
		while(true){
			lv_indev_read(indev->indev);
			std::lock_guard<std::mutex>lock(indev->mutex);
			if(indev->queue.empty())break;
		}
		indev->reading=false;
	};
	auto start_read=[&](evdev_indev&indev){
		bool want=false;
		{
			std::lock_guard<std::mutex>lock(indev.mutex);
			want=!indev.queue.empty();
		}
		if(want&&!indev.reading){
			indev.reading=true;
			lv_lock();
			lv_async_call(do_read,&indev);
			lv_unlock();
		}
	};
	start_read(encoder);
	start_read(keypad);
	start_read(pointer);
}
