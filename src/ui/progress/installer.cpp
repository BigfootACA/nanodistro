#include"internal.h"
#include"fs-utils.h"
#include"path-utils.h"
#include"configs.h"
#include"log.h"

void ui_draw_progress::invoke_children(){
	try{
		std::vector exc{0,1,2,status_fd[1]};
		fs_close_all_fds(exc);
		setenv("TERM","xterm-256color",1);
		setenv("COLORTERM","tsm",1);
		setenv("NANODISTRO_STATUS_FD",std::to_string(status_fd[1]).c_str(),1);
		setenv("NANODISTRO_INSTALLER_CFG",installer_file.c_str(),1);
		auto self=path_get_self();
		execl(self.c_str(),"flasher",installer_file.c_str(),nullptr);
	}catch(...){}
	_exit(1);
}

void ui_draw_progress::on_exited(bool success){
	if(state!=STATE_RUNNING&&state!=STATE_CANCEL)return;
	if(state==STATE_CANCEL)success=false;
	set_state(success?STATE_SUCCESS:STATE_FAILED);
	stop_terminal();
}

void ui_draw_progress::write_installer_file(){
	int v=0;
	Json::StyledWriter writer{};
	do{
		installer_file=std::format("/tmp/.nanodistro-{}-installer-{}.json",getpid(),v);
	}while(fs_exists(installer_file));
	auto data=writer.write(installer_context);
	fs_write_all(installer_file,data);
	log_info("wrote installer config file {}",installer_file);
	log_debug("config: {}",data);
}
