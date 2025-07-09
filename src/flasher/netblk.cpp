#include<sys/wait.h>
#include"internal.h"
#include"fs-utils.h"
#include"path-utils.h"
#include"configs.h"
#include"status.h"
#include"url.h"
#include"error.h"

static pid_t run_netblk(const std::string&in,const std::string&out){
	if(flasher.netblk)throw RuntimeError("netblk already running");
	flasher.netblk=std::make_shared<process>();
	flasher.netblk->exe=path_get_self();
	flasher.netblk->args={"netblk",in,out};
	flasher.netblk->set_fd(installer_status_fd,installer_status_fd);
	flasher.netblk->start();
	if(flasher.netblk->pid<0)
		throw RuntimeError("failed to start netblk");
	return flasher.netblk->pid;
}

static Json::Value generate_netblk_config(){
	url target;
	Json::Value json;
	auto images_url_s=installer_context["images-url"].asString();
	if(images_url_s.empty())throw RuntimeError("images-url not set");
	url images_url(images_url_s);
	auto file_url_s=installer_context["image"]["file_url"].asString();
	if(file_url_s.empty())throw RuntimeError("file_url not set");
	url file_url=images_url.relative(file_url_s);
	log_info("netblk file url {}",file_url.get_url());
	json=installer_context["config"]["netblk"];
	if(!json["backend"].isMember("driver"))
		json["backend"]["driver"]="curl-readcache";
	json["backend"]["url"]=file_url.to_string();
	return json;
}

static void write_netblk_config(const std::string&path){
	Json::StyledWriter writer{};
	auto json=generate_netblk_config();
	auto data=writer.write(json);
	fs_write_all(path,data);
	log_info("wrote netblk config file {}",path);
}

static std::string choose_temp(const std::string&type){
	size_t i=0;
	std::string path{};
	do{
		path=std::format("/tmp/.nanodistro-{}-{}-{}.json",getpid(),type,i++);
	}while(fs_exists(path));
	return path;
}

static Json::Value read_netblk_config(const std::string&path){
	Json::Value json;
	Json::Reader reader{};
	auto data=fs_read_all(path);
	if(!reader.parse(data,json))throw RuntimeError(
		"failed to parse netblk config: {}",
		reader.getFormattedErrorMessages()
	);
	return json;
}

void start_netblk(){
	auto path_in=choose_temp("netblk-in");
	auto path_out=choose_temp("netblk-out");
	write_netblk_config(path_in);
	auto pid=run_netblk(path_in,path_out);
	if(pid<0)throw RuntimeError("failed to run netblk");
	log_info("started netblk as pid {}",pid);
	int st=0;
	do{
		if(waitpid(pid,&st,WNOHANG)==pid){
			if(WIFSIGNALED(st))throw RuntimeError("netblk exited by signal {}",WTERMSIG(st));
			if(WIFEXITED(st))throw RuntimeError("netblk exited with status {}",WEXITSTATUS(st));
			throw RuntimeError("netblk exited unexpectedly");
		}
		usleep(300000);
	}while(!fs_exists(path_out));
	log_info("found netblk config {}",path_out);
	auto result=read_netblk_config(path_out);
	auto blk=result["device"]["path"].asString();
	if(!fs_is_block(blk))throw RuntimeError("netblk device {} not found",blk);
	log_info("netblk device initialized at {}",blk);
	unlink(path_in.c_str());
	unlink(path_out.c_str());
	flasher.img_device=blk;
	flasher.netblk_result=result;
}
