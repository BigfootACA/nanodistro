#include"internal.h"
#include"log.h"
#include"str-utils.h"

void ui_draw_progress::proc_status(){
	char buff[4096];
	while(true){
		auto ret=read(status_fd[0],buff,sizeof(buff));
		if(ret<=0){
			if(ret<0&&errno==EINTR)return;
			if(ret<0&&errno==EAGAIN)return;
			if(ret==0)log_warning("read status reached EOF");
			else log_warning("read status failed: {}",strerror(errno));
			close(status_fd[0]);
			close(status_fd[1]);
			status_fd[0]=-1;
			status_fd[1]=-1;
			return;
		}
		std::string str(buff,buff+ret);
		for(auto&line:str_split(str,'\n')){
			if(line.empty())continue;
			auto pos=line.find(' ');
			if(pos==std::string::npos)continue;
			auto key=line.substr(0,pos);
			auto val=line.substr(pos+1);
			proc_one_status(key,val);
		}
	}
}

void ui_draw_progress::proc_one_status(const std::string&key,const std::string&val){
	if(key=="set_status_text"){
		lv_lock();
		lv_label_set_text(lbl_report,val.c_str());
		lv_unlock();
	}else if(key=="set_progress_enable"){
		bool en=false;
		if(string_is_false(val))en=false;
		else if(string_is_true(val))en=true;
		else return;
		lv_lock();
		set_progress(en,progress_value);
		lv_unlock();
	}else if(key=="set_progress_value"){
		size_t idx=0;
		auto prog=std::stoi(val,&idx);
		if(idx!=val.length())return;
		lv_lock();
		set_progress(progress_enable,prog);
		lv_unlock();
	}else if(key=="log"){
		Json::Value jval;
		Json::Reader reader;
		if(!reader.parse(val,jval)){
			log_warning(
				"parse log failed: {}",
				reader.getFormattedErrorMessages()
			);
			return;
		}
		log_print_json(jval);
	}
}
