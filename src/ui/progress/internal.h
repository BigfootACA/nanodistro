#ifndef PROGRESS_INTERNAL_H
#define PROGRESS_INTERNAL_H
#include<lvgl.h>
#include<queue>
#include<thread>
#include<semaphore>
#include"../ui.h"
#include"termview.h"
extern "C"{
#include"shl-pty.h"
}

enum progress_state{
	STATE_RUNNING,
	STATE_SUCCESS,
	STATE_FAILED,
	STATE_CANCEL,
};

class ui_draw_progress:public ui_draw{
	public:
		~ui_draw_progress()override;
		void test(lv_timer_t*);
		void draw(lv_obj_t*cont)override;
		void draw_buttons();
		void btn_cancel_cb(lv_event_t*);
		void btn_back_cb(lv_event_t*);
		void btn_log_cb(lv_event_t*);
		void on_exited(bool success);
		void write_installer_file();
		void init_terminal();
		void start_terminal();
		void stop_terminal();
		void pty_dispatch_thread(std::stop_token stop);
		void pty_dispatch_task();
		void proc_status();
		void proc_one_status(const std::string&key,const std::string&val);
		void invoke_children();
		void set_termview_show(bool show);
		void set_state(progress_state state);
		void set_progress(bool en,int val);
		void log_proc();
		lv_timer_t*log_timer=nullptr;
		std::queue<std::string>log_queue{};
		std::mutex log_mutex{};
		bool progress_enable=false;
		int progress_value=0;
		bool termview_show=false;
		progress_state state;
		lv_obj_t*box=nullptr;
		lv_obj_t*btn_cancel=nullptr;
		lv_obj_t*btn_back=nullptr;
		lv_obj_t*btn_log=nullptr;
		lv_obj_t*termview=nullptr;
		lv_obj_t*spinner=nullptr;
		lv_obj_t*lbl_progress=nullptr;
		lv_obj_t*lbl_status=nullptr;
		lv_obj_t*lbl_report=nullptr;
		lv_obj_t*lbl_success=nullptr;
		lv_obj_t*lbl_failed=nullptr;
		shl_pty*pty=nullptr;
		std::jthread thread{};
		std::binary_semaphore sem{0};
		pid_t pty_pid=-1;
		int pty_bridge=-1;
		int status_fd[2]{-1,-1};
		std::string installer_file{};
		uint64_t log_hook=0;
};

#endif
