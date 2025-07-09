#include<fcntl.h>
#include<csignal>
#include<sys/wait.h>
#include"internal.h"
#include"error.h"
#include"log.h"
#include"gui.h"

static void terminal_read_cb(
	shl_pty*pty,
	void*data,
	char*u8,
	size_t len
){
	auto ui=(ui_draw_progress*)data;
	if(!ui||!pty||ui->pty!=pty)return;
	auto vte=lv_termview_get_vte(ui->termview);
	tsm_vte_input(vte,u8,len);
	lv_termview_update(ui->termview);
}

void ui_draw_progress::pty_dispatch_task(){
	shl_pty_bridge_dispatch(pty_bridge,0);
	lv_termview_update(termview);
	sem.release();
}

void ui_draw_progress::init_terminal(){
	lv_termview_set_write_cb(termview,[pty=pty](auto tv,auto u8,auto len){
		if(pty){
			shl_pty_write(pty,u8,len);
			shl_pty_dispatch(pty);
		}
	});
	lv_termview_set_resize_cb(termview,[pty=pty](auto tv,auto cols,auto rows){
		if(pty)shl_pty_resize(pty,cols,rows);
	});
	auto ui_small=fonts_get("ui-small");
	auto term_normal=fonts_get("term-normal");
	auto term_bold=fonts_get("term-bold");
	auto term_italic=fonts_get("term-italic");
	auto term_bold_italic=fonts_get("term-bold-italic");
	if(!term_normal)term_normal=ui_small;
	if(!term_bold)term_bold=term_normal;
	if(!term_italic)term_italic=term_normal;
	if(!term_bold_italic)term_bold_italic=term_normal;
	if(!term_normal)throw RuntimeError("failed to get font");
	lv_termview_set_font_regular(termview,term_normal);
	lv_termview_set_font_bold(termview,term_bold);
	lv_termview_set_font_italic(termview,term_italic);
	lv_termview_set_font_bold_italic(termview,term_bold_italic);
}

void ui_draw_progress::stop_terminal(){
	if(pty_pid>0){
		kill(pty_pid,SIGKILL);
		waitpid(pty_pid,nullptr,0);
	}
	if(thread.joinable())
		thread.request_stop();
	if(thread.joinable())
		thread.join();
	if(pty_bridge>0){
		if(pty)shl_pty_bridge_remove(pty_bridge,pty);
		shl_pty_bridge_free(pty_bridge);
	}
	if(log_timer){
		lv_async_call_func(std::bind(&ui_draw_progress::log_proc,this));
		lv_timer_delete(log_timer);
		log_timer=nullptr;
	}
	if(status_fd[0]>=0)close(status_fd[0]);
	if(status_fd[1]>=0)close(status_fd[1]);
	if(!installer_file.empty())unlink(installer_file.c_str());
	if(pty)shl_pty_unref(pty);
	if(log_hook!=0)log_del_print_string_hook(log_hook);
	status_fd[0]=-1;
	status_fd[1]=-1;
	pty_pid=-1;
	pty_bridge=-1;
	pty=nullptr;
	log_hook=0;
}

void ui_draw_progress::start_terminal(){
	int ret;
	stop_terminal();
	lv_label_set_text(lbl_report,_("Starting installer..."));
	if(pipe(status_fd)<0)throw ErrnoError("create pipe failed");
	fcntl(status_fd[0],F_SETFL,fcntl(status_fd[0],F_GETFL)|O_NONBLOCK);
	write_installer_file();
	if((pty_bridge=shl_pty_bridge_new())<0)
		throw ErrnoErrorWith(pty_bridge,"create pty bridge failed");
	pty_pid=shl_pty_open(
		&pty,
		terminal_read_cb,this,
		lv_termview_get_cols(termview),
		lv_termview_get_rows(termview)
	);
	if(pty_pid<0)throw ErrnoErrorWith(-pty_pid,"create pty failed");
	if(pty_pid==0)invoke_children();
	if((ret=shl_pty_bridge_add(pty_bridge,pty))<0)
		throw ErrnoErrorWith(ret,"add pty to bridge failed");
	auto f=std::bind(&ui_draw_progress::pty_dispatch_thread,this,std::placeholders::_1);
	thread=std::jthread(f);
	log_hook=log_add_print_string_hook([this](auto log){
		std::lock_guard<std::mutex>lock(log_mutex);
		log_queue.push(log);
		return true;
	});
	log_timer=lv_timer_create([](auto v){
		auto ui=(ui_draw_progress*)lv_timer_get_user_data(v);
		if(ui)ui->log_proc();
	},100,this);
}

void ui_draw_progress::log_proc(){
	std::string buff{};
	std::lock_guard<std::mutex>lock(log_mutex);
	while(!log_queue.empty()){
		auto log=log_queue.front();
		log_queue.pop();
		if(!buff.empty())buff+="\r\n";
		buff+=log;
	}
	if(!buff.empty())lv_termview_line_prints(termview,buff);
}
