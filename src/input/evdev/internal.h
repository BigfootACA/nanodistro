#ifndef INTERNAL_H
#define INTERNAL_H
#include<atomic>
#include<queue>
#include<linux/input.h>
#include"../internal.h"
#include"event-loop.h"

union evdev_bits{
	struct{
		uint8_t syn[8];
		uint8_t key[128];
		uint8_t rel[8];
		uint8_t abs[8];
		uint8_t msc[8];
		uint8_t sw[8];
		uint8_t led[8];
		uint8_t snd[8];
		uint8_t ff[8];
	}d;
	struct{
		uint64_t syn;
		uint8_t key[128];
		uint64_t rel;
		uint64_t abs;
		uint64_t msc;
		uint64_t sw;
		uint64_t led;
		uint64_t snd;
		uint64_t ff;
	}v;
};

struct pointer_report{
	uint32_t x=0;
	uint32_t y=0;
	bool press=false;
};

struct keypad_report{
	uint32_t key=0;
	bool press=false;
};

struct encoder_report{
	int32_t movement=0;
};

struct touchpad_finger{
	int64_t last_x=-1,last_y=-1;
	int64_t move_x=0,move_y=0;
	time_t last_touch=0;
	bool active=false;
};

class evdev_context;
class evdev_device{
	public:
		evdev_device(evdev_context*ctx):ctx(ctx){}
		~evdev_device();
		void init_device();
		void on_event(event_handler_context*ev);
		bool read_event();
		void cleanup_event();
		void process_event(input_event&ev);
		void process_led(input_event&ev);
		void process_abs(input_event&ev);
		void process_rel(input_event&ev);
		void process_key(input_event&ev);
		void process_keyboard(input_event&ev);
		void stop();
		uint64_t poll_id=0;
		evdev_context*ctx;
		std::string path{};
		std::string name{};
		evdev_bits bits{};
		input_absinfo abs[ABS_MAX]{};
		touchpad_finger finger[10]{};
		touchpad_finger vfinger{};
		time_t last_touch=0;
		int64_t last_avg_y=-1;
		int32_t scroll_accum=0;
		bool pressing=false;
		int mt_slot=0;
		uint64_t props=0;
		int fd=-1;
};

union report{
	keypad_report k;
	pointer_report p;
	encoder_report e;
};

struct evdev_indev{
	lv_indev_t*indev=nullptr;
	std::queue<report>queue{};
	report last{};
	std::atomic<bool>reading=false;
	std::mutex mutex{};
	void add(const keypad_report&report);
	void add(const pointer_report&report);
	void add(const encoder_report&report);
};

class evdev_context{
	public:
		std::vector<evdev_device*>probe_all();
		void init();
		void init_indev();
		void draw_mouse();
		void handle_tty(int tty);
		void trigger_read();
		void pointer_read(lv_indev_t*indev,lv_indev_data_t*data);
		void keypad_read(lv_indev_t*indev,lv_indev_data_t*data);
		void encoder_read(lv_indev_t*indev,lv_indev_data_t*data);
		void apply(const YAML::Node&cfg);
		bool process_uevent(uint64_t id,std::map<std::string,std::string>&uevent);
		evdev_device*start(const std::string&dev);
		void stop(const std::string&dev);
		uint64_t uevent_id=0;
		std::vector<evdev_device*>devices{};
		lv_obj_t*mouse=nullptr;
		evdev_indev encoder{},keypad{},pointer{};
		pointer_report pointer_val{};
		bool detect_mouse=false;
		bool key_capslock=false;
		bool key_numlock=false;
		bool key_scrolllock=false;
		bool key_shift=false;
		bool key_ctrl=false;
		bool key_alt=false;
		bool key_meta=false;
};

class input_backend_evdev:public input_backend{
	public:
		std::vector<lv_indev_t*>init(const YAML::Node&cfg)override;
};

extern std::shared_ptr<evdev_context>evdev_ctx;
#endif
