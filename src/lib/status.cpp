#include"status.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"path-utils.h"
#include"json-utils.h"
#include"log.h"
#include"configs.h"
#include"error.h"

int installer_status_fd=-1;
static std::mutex mutex{};

void installer_write_cmd(const std::string&cmd){
	if(installer_status_fd<0)return;
	std::lock_guard<std::mutex>lock(mutex);
	full_write(installer_status_fd,cmd.c_str(),cmd.length());
	fdatasync(installer_status_fd);
}

int installer_init(){
	size_t idx=0;
	auto val=str_get_env("NANODISTRO_STATUS_FD");
	if(val.empty())return 0;
	installer_status_fd=std::stoi(val,&idx);
	if(idx!=val.length())throw InvalidArgument("invalid status fd");
	if(!fs_is_link(path_join("/proc/self/fd",std::to_string(installer_status_fd))))
		throw InvalidArgument("invalid status fd");
	return 0;
}

void installer_set_progress_enable(bool enable){
	installer_write_cmd(std::format("set_progress_enable {}\n",enable?"true":"false"));
}

void installer_set_progress_value(int value){
	value=std::clamp(value,0,100);
	installer_write_cmd(std::format("set_progress_value {}\n",value));
}

void installer_set_status(const std::string&value){
	std::string val=value;
	str_trim(val);
	if(auto v=val.find('\n');v!=std::string::npos)
		val.erase(v,std::string::npos);
	installer_write_cmd(std::format("set_status_text {}\n",val));
}

void installer_load_context(const std::string&path){
	std::string rpath=path;
	Json::Reader reader;
	if(rpath.empty())rpath=str_get_env("NANODISTRO_INSTALLER_CFG");
	if(rpath.empty())throw InvalidArgument("no installer config file");
	config.reset();
	installer_context.clear();
	auto cont=fs_read_all(rpath);
	auto ret=reader.parse(cont,installer_context);
	if(!ret)throw RuntimeError(
		"failed to parse installer config file: {}",
		reader.getFormattedErrorMessages()
	);
	if(installer_context.isMember("config"))
		json_to_yaml(config,installer_context["config"]);
	log_info("loaded installer config file {}",rpath);
}
