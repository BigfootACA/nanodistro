#include<fcntl.h>
#include<signal.h>
#include<dirent.h>
#include<unistd.h>
#include<linux/vt.h>
#include<linux/kd.h>
#include<sys/ioctl.h>
#include"path-utils.h"
#include"fs-utils.h"
#include"gui.h"
#include"log.h"
#include"worker.h"
#include"internal.h"
#include"error.h"

static int use_tty=0;

int open_tty(int tty){
	if(tty<0)throw InvalidArgument("tty {} is invalid",tty);
	auto file=std::format("/dev/tty{}",tty);
	if(!fs_exists(file))throw RuntimeError("tty {} not found",file);
	int fd=open(file.c_str(),O_RDWR|O_CLOEXEC);
	if(fd<0)throw ErrnoError("open tty {} failed",file);
	return fd;
}

void set_console(int con){
	auto fd=open_tty(0);
	if(ioctl(fd,VT_ACTIVATE,con)>=0)
		ioctl(fd,VT_WAITACTIVE,con);
	close(fd);
}

void bind_vtconsole(int value){
	std::string dir="/sys/class/vtconsole";
	auto d=opendir(dir.c_str());
	if(!d)return;
	while(auto e=readdir(d))try{
		if(e->d_type!=DT_LNK||e->d_name[0]=='.')continue;
		std::string dname=e->d_name;
		auto path=path_join(dir,dname);
		if(!dname.starts_with("vtcon"))continue;
		auto p_name=path_join(path,"name");
		auto name=fs_simple_read(p_name);
		if(name.ends_with("dummy device"))continue;
		auto p_bind=path_join(path,"bind");
		fs_write_numlf(p_bind,value);
	}catch(...){}
	closedir(d);
}

static void set_display_mode(bool gui){
	int fd=open_tty(use_tty);
	if(gui){
		log_info("switch to GUI mode");
		ioctl(fd,KDSETMODE,KD_GRAPHICS);
		bind_vtconsole(0);
		bool want_flush=gui_pause;
		gui_pause=false;
		if(want_flush&&cur_display)
			cur_display->force_flush();
	}else{
		log_info("switch to text mode");
		gui_pause=true;
		if(cur_display)
			cur_display->fill_color(lv_color_black());
		bind_vtconsole(1);
		ioctl(fd,KDSETMODE,KD_TEXT);
	}
	close(fd);
}

void display_switch_gui(){
	if(use_tty<=0)return;
	set_console(use_tty);
	set_display_mode(true);
}

void display_switch_tty(int tty){
	if(use_tty<=0)return;
	if(tty==use_tty){
		display_switch_gui();
	}else{
		set_display_mode(false);
		set_console(1);
	}
}

void display_setup_console(int tty){
	use_tty=tty;
	if(use_tty<=0)return;
	vt_mode vt_mode{
		.mode=VT_PROCESS,
		.relsig=SIGUSR1,
		.acqsig=SIGUSR2,
	};
	int fd=open_tty(tty);
	ioctl(fd,VT_SETMODE,&vt_mode);
	close(fd);
	display_switch_gui();
}

void display_deinit_console(){
	if(use_tty<=0)return;
	display_switch_tty(1);
}

// static void display_signal(){
// 	int fd=open_tty(0);
// 	vt_stat vts{};
// 	if(ioctl(fd,VT_GETSTATE,&vts)>=0){
// 		set_display_mode(vts.v_active==use_tty);
// 	}
// 	close(fd);
// }

void display_handle_signal(int sig){
	if(use_tty<=0)return;
	int fd=open_tty(use_tty);
	if(sig==SIGUSR1){
		set_display_mode(false);
		ioctl(fd,VT_RELDISP,1);
	}else if(sig==SIGUSR2){
		set_display_mode(true);
	}
	close(fd);
	// worker_add(display_signal);
}
