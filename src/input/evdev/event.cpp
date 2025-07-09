#include"internal.h"
#include"log.h"

void evdev_device::process_led(input_event&ev){
	switch(ev.code){
		case LED_CAPSL:ctx->key_capslock=!!ev.value;break;
		case LED_NUML:ctx->key_numlock=!!ev.value;break;
		case LED_SCROLLL:ctx->key_scrolllock=!!ev.value;break;
		default:;
	}
}

static inline int64_t scale(
	int64_t in_val,
	int64_t in_min,int64_t in_max,
	int64_t out_min,int64_t out_max
){
	if(in_min>=in_max||out_min>out_max||in_max==0||out_max==0)return 0;
	return (in_val-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;
}

void evdev_device::process_abs(input_event&ev){
	auto w=lv_disp_get_hor_res(nullptr);
	auto h=lv_disp_get_ver_res(nullptr);
	int64_t min=abs[ev.code].minimum;
	int64_t max=abs[ev.code].maximum;
	int64_t res=std::max(1,abs[ev.code].resolution);
	if(max==0||min>max)return;
	switch(ev.code){
		case ABS_X:{
			if(props!=0&&!((props>>INPUT_PROP_DIRECT)&1))break;
			ctx->pointer_val.x=scale(ev.value,min,max,0,w-1);
			ctx->pointer.add(ctx->pointer_val);
			ctx->detect_mouse=!((props>>INPUT_PROP_DIRECT)&1);
		}break;
		case ABS_Y:{
			if(props!=0&&!((props>>INPUT_PROP_DIRECT)&1))break;
			ctx->pointer_val.y=scale(ev.value,min,max,0,h-1);
			ctx->pointer.add(ctx->pointer_val);
			ctx->detect_mouse=!((props>>INPUT_PROP_DIRECT)&1);
		}break;
		case ABS_MT_SLOT:mt_slot=ev.value;break;
		case ABS_MT_TRACKING_ID:finger[mt_slot].active=ev.value>=0;break;
		case ABS_MT_POSITION_X:{
			if(!((props>>INPUT_PROP_POINTER)&1))break;
			ctx->detect_mouse=true;
			if(mt_slot==0){
				auto cx=scale(ev.value,min,max,0,w);
				if(vfinger.last_x>=0){
					auto lx=scale(vfinger.last_x,min,max,0,w);
					int64_t px=ctx->pointer_val.x+cx-lx;
					ctx->pointer_val.x=std::clamp<int64_t>(px,0,w-1);
					ctx->pointer.add(ctx->pointer_val);
					vfinger.move_x+=ev.value-vfinger.last_x;
				}
				vfinger.last_x=ev.value;
			}else if(mt_slot<10){
				auto&f=finger[mt_slot];
				if(f.last_x>=0)
					f.move_x+=ev.value-f.last_x;
				f.last_x=ev.value;
			}

		}break;
		case ABS_MT_POSITION_Y:{
			if(!((props>>INPUT_PROP_POINTER)&1))break;
			ctx->detect_mouse=true;
			auto cy=scale(ev.value,min,max,0,h);
			if(mt_slot==0&&!finger[1].active){
				if(vfinger.last_y>=0){
					auto ly=scale(vfinger.last_y,min,max,0,h);
					int64_t py=ctx->pointer_val.y+cy-ly;
					ctx->pointer_val.y=std::clamp<int64_t>(py,0,h-1);
					ctx->pointer.add(ctx->pointer_val);
					vfinger.move_y+=ev.value-vfinger.last_y;
				}
				vfinger.last_y=ev.value;
			}
			if(mt_slot<10){
				auto&f=finger[mt_slot];
				if(f.last_y>=0)
					f.move_y+=ev.value-f.last_y;
				f.last_y=ev.value;
			}
			if(finger[0].active&&finger[1].active&&!finger[2].active){
				int64_t avg_y=(finger[0].last_y+finger[1].last_y)/2;
				if(last_avg_y!=-1){
					int32_t mv=0;
					auto th=std::min<int64_t>(32,res*8);
					scroll_accum+=avg_y-last_avg_y;
					while(scroll_accum>th)scroll_accum-=th,mv--;
					while(scroll_accum<-th)scroll_accum+=th,mv++;
					if(mv!=0)ctx->encoder.add({.movement=mv});
				}
				last_avg_y=avg_y;
			}else last_avg_y=-1,scroll_accum=0;
		}break;
		default:;
	}
}

void evdev_device::process_rel(input_event&ev){
	auto w=lv_disp_get_hor_res(nullptr);
	auto h=lv_disp_get_ver_res(nullptr);
	switch(ev.code){
		case REL_X:{
			int32_t v=(int32_t)ctx->pointer_val.x+ev.value;
			ctx->pointer_val.x=std::clamp(v,0,w-1);
			ctx->pointer.add(ctx->pointer_val);
			ctx->detect_mouse=true;
		}break;
		case REL_Y:{
			int32_t v=(int32_t)ctx->pointer_val.y+ev.value;
			ctx->pointer_val.y=std::clamp(v,0,h-1);
			ctx->pointer.add(ctx->pointer_val);
			ctx->detect_mouse=true;
		}break;
		case REL_WHEEL:{
			if((bits.v.rel>>REL_WHEEL_HI_RES)&1)break;
			ctx->encoder.add({.movement=-ev.value});
		}break;
		case REL_WHEEL_HI_RES:{
			ctx->encoder.add({.movement=-ev.value/120});
		}break;
		default:;
	}
}

void evdev_device::process_keyboard(input_event&ev){
	uint32_t key=0;
	if(ev.code==KEY_LEFTSHIFT||ev.code==KEY_RIGHTSHIFT)
		ctx->key_shift=!ev.value;
	if(ev.code==KEY_LEFTCTRL||ev.code==KEY_RIGHTCTRL)
		ctx->key_ctrl=!ev.value;
	if(ev.code==KEY_LEFTALT||ev.code==KEY_RIGHTALT)
		ctx->key_alt=!ev.value;
	if(ev.code==KEY_LEFTMETA||ev.code==KEY_RIGHTMETA)
		ctx->key_meta=!ev.value;
	if(ev.code>=KEY_A&&ev.code<=KEY_Z){
		bool upper=false;
		if(ctx->key_shift)upper=!upper;
		if(ctx->key_capslock)upper=!upper;
		key=ev.code-KEY_A+(upper?'A':'a');
	}
	switch(ev.code){
		case KEY_GRAVE:key=ctx->key_shift?'~':'`';break;
		case KEY_1:key=ctx->key_shift?'!':'1';break;
		case KEY_2:key=ctx->key_shift?'@':'2';break;
		case KEY_3:key=ctx->key_shift?'#':'3';break;
		case KEY_4:key=ctx->key_shift?'$':'4';break;
		case KEY_5:key=ctx->key_shift?'%':'5';break;
		case KEY_6:key=ctx->key_shift?'^':'6';break;
		case KEY_7:key=ctx->key_shift?'&':'7';break;
		case KEY_8:key=ctx->key_shift?'*':'8';break;
		case KEY_9:key=ctx->key_shift?'(':'9';break;
		case KEY_0:key=ctx->key_shift?')':'0';break;
		case KEY_MINUS:key=ctx->key_shift?'_':'-';break;
		case KEY_EQUAL:key=ctx->key_shift?'+':'=';break;
		case KEY_LEFTBRACE:key=ctx->key_shift?'{':'[';break;
		case KEY_RIGHTBRACE:key=ctx->key_shift?'}':']';break;
		case KEY_BACKSLASH:key=ctx->key_shift?'|':'\\';break;
		case KEY_SEMICOLON:key=ctx->key_shift?':':';';break;
		case KEY_APOSTROPHE:key=ctx->key_shift?'\"':'\'';break;
		case KEY_COMMA:key=ctx->key_shift?'<':',';break;
		case KEY_DOT:key=ctx->key_shift?'>':'.';break;
		case KEY_SLASH:key=ctx->key_shift?'?':'/';break;
		case KEY_SPACE:key=' ';break;
		case KEY_UP:key=LV_KEY_UP;break;
		case KEY_DOWN:key=LV_KEY_DOWN;break;
		case KEY_RIGHT:key=LV_KEY_RIGHT;break;
		case KEY_LEFT:key=LV_KEY_LEFT;break;
		case KEY_ESC:key=LV_KEY_ESC;break;
		case KEY_DELETE:key=LV_KEY_DEL;break;
		case KEY_BACKSPACE:key=LV_KEY_BACKSPACE;break;
		case KEY_ENTER:key=LV_KEY_ENTER;break;
		case KEY_NEXT:key=LV_KEY_NEXT;break;
		case KEY_TAB:key=LV_KEY_NEXT;break;
		case KEY_PREVIOUS:key=LV_KEY_PREV;break;
		case KEY_HOME:key=LV_KEY_HOME;break;
		case KEY_END:key=LV_KEY_END;break;
	}
	int tty=0;
	if(ctx->key_ctrl&&ctx->key_alt)switch(ev.code){
		case KEY_F1:tty=1;break;
		case KEY_F2:tty=2;break;
		case KEY_F3:tty=3;break;
		case KEY_F4:tty=4;break;
		case KEY_F5:tty=5;break;
		case KEY_F6:tty=6;break;
		case KEY_F7:tty=7;break;
		case KEY_F8:tty=8;break;
		case KEY_F9:tty=9;break;
		case KEY_F10:tty=10;break;
		case KEY_F11:tty=11;break;
		case KEY_F12:tty=12;break;
	}
	if(tty>0&&ev.value==1){
		ctx->handle_tty(tty);
		return;
	}
	if(key!=0&&!ctx->key_alt&&!ctx->key_ctrl&&!ctx->key_meta){
		keypad_report report{};
		report.key=key;
		report.press=!!ev.value;
		ctx->keypad.add(report);
	}
}

void evdev_device::process_key(input_event&ev){
	switch(ev.code){
		case BTN_TOUCH:{
			if(ev.value==1)last_touch=ev.input_event_sec;
			if(((props>>INPUT_PROP_POINTER)&1)){
				auto resx=std::min(1,abs[ABS_MT_POSITION_X].resolution);
				auto resy=std::min(1,abs[ABS_MT_POSITION_Y].resolution);
				if(ev.value!=0)break;
				if(
					ev.input_event_sec-last_touch<2&&
					vfinger.move_x>-resx&&vfinger.move_x<resx&&
					vfinger.move_y>-resy&&vfinger.move_y<resy
				){
					ctx->pointer_val.press=true;
					ctx->pointer.add(ctx->pointer_val);
					ctx->pointer_val.press=false;
					ctx->pointer.add(ctx->pointer_val);
				}
				vfinger.move_x=0,vfinger.move_y=0;
				vfinger.last_x=-1,vfinger.last_y=-1;
			}else{
				pressing=!!ev.value;
				ctx->pointer_val.press=pressing;
				ctx->pointer.add(ctx->pointer_val);
			}
		}break;
		case BTN_MIDDLE:{
			keypad_report report{};
			report.key=LV_KEY_ENTER;
			report.press=!!ev.value;
			ctx->keypad.add(report);
		}break;
		case BTN_LEFT:
			pressing=!!ev.value;
			ctx->pointer_val.press=pressing;
			ctx->pointer.add(ctx->pointer_val);
		break;
		default:process_keyboard(ev);break;
	}
}

void evdev_device::process_event(input_event&ev){
	switch(ev.type){
		case EV_LED:process_led(ev);break;
		case EV_ABS:process_abs(ev);break;
		case EV_REL:process_rel(ev);break;
		case EV_KEY:process_key(ev);break;
		default:;
	}
	static constexpr size_t max_event=16384;
	bool overrun=false,warned=false;
	{
		std::lock_guard<std::mutex>lock(ctx->encoder.mutex);
		while(ctx->encoder.queue.size()>max_event)
			ctx->encoder.queue.pop(),overrun=true;
	}{
		std::lock_guard<std::mutex>lock(ctx->keypad.mutex);
		while(ctx->keypad.queue.size()>max_event)
			ctx->keypad.queue.pop(),overrun=true;
	}{
		std::lock_guard<std::mutex>lock(ctx->pointer.mutex);
		while(ctx->pointer.queue.size()>max_event)
			ctx->pointer.queue.pop(),overrun=true;
	}
	if(overrun&&!warned){
		log_warning("input event queue overrun");
		warned=true;
	}
}

void evdev_device::cleanup_event(){
	if(pressing&&!ctx->pointer_val.press){
		ctx->pointer_val.press=false;
		ctx->pointer.add(ctx->pointer_val);
	}
}
