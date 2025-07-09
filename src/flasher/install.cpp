#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/utsname.h>
#include"internal.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"std-utils.h"
#include"path-utils.h"
#include"libmounts.h"
#include"cleanup.h"
#include"status.h"
#include"log.h"
#include"error.h"

flasher_context flasher{};

static void prepare_image(){
	auto format=installer_context["image"]["format"].asString();
	if(format=="mount-run")start_netblk();
	else throw RuntimeError("unsupported image format {}",format);
}

static void choose_folder(){
	size_t i=0;
	do{
		flasher.folder=std::format(
			"/tmp/nanodistro-flasher-{}-{}",
			getpid(),i++
		);
	}while(fs_is_folder(flasher.folder));
	auto ret=mkdir(flasher.folder.c_str(),0711);
	if(ret<0)throw ErrnoError("failed to create nanodistro folder");
	auto mnt_dir=path_join(flasher.folder,"mnt");
	auto bin_dir=path_join(flasher.folder,"bin");
	mkdir(mnt_dir.c_str(),0700);
	mkdir(bin_dir.c_str(),0711);
	auto self=path_get_self();
	symlink(self.c_str(),path_join(bin_dir,"flasher").c_str());
	symlink(self.c_str(),path_join(bin_dir,"netblk").c_str());
	symlink(self.c_str(),path_join(bin_dir,"helper").c_str());
}

static void mount_image(){
	int ret;
	auto mnt=path_join(flasher.folder,"mnt");
	auto ctx=mnt_new_context();
	if(!ctx)throw RuntimeError("failed to create mount context");
	cleanup_func mount_ctx(std::bind(mnt_free_context,ctx));
	mnt_context_set_target(ctx,mnt.c_str());
	mnt_context_set_source(ctx,flasher.img_device.c_str());
	mnt_context_set_fstype(ctx,"auto");
	mnt_context_set_options(ctx,"ro");
	log_info("mounting {} to {}",flasher.img_device,mnt);
	if((ret=mnt_context_mount(ctx))<0)throw RuntimeError(
		"failed to mount {}",flasher.img_device,libmount_strerror(ret)
	);
	chdir(mnt.c_str());
}

static void prepare_image_mnt(){
	prepare_image();
	if(flasher.img_device.empty())
		throw RuntimeError("image device not set");
	if(!fs_is_block(flasher.img_device))throw RuntimeError(
		"image device {} is not a block device",flasher.img_device
	);
	mount_image();
}

static void load_image_manifest(){
	auto mnt=path_join(flasher.folder,"mnt");
	auto manifest=path_join(mnt,"manifest.yaml");
	if(!fs_exists(manifest))throw RuntimeError(
		"image manifest {} not found",manifest
	);
	try{
		flasher.img_manifest=YAML::LoadFile(manifest);
	}catch(...){
		log_error("failed to parse image manifest {}",manifest);
		throw;
	}
	log_info("loaded image manifest {}",manifest);
}

static bool check_arch(const std::string&id,YAML::Node&node){
	auto arch=node["arch"].as<std::string>();
	if(!node["arch"])return true;
	std::vector<std::string>archs{};
	if(node["arch"].IsScalar())archs.push_back(arch);
	else if(node["arch"].IsSequence())
		for(auto v:node["arch"])if(v.IsScalar())
			archs.push_back(v.as<std::string>());
	if(archs.empty())return true;
	for(auto&v:archs)if(v=="any")return true;
	utsname u{};
	if(uname(&u)<0)throw ErrnoError("failed to get utsname");
	for(auto&v:archs)if(v==u.machine)return true;
	log_info("skip {} because arch {} incompatible",id,u.machine);
	return false;
}

static YAML::Node choose_script(){
	auto scripts=flasher.img_manifest["manifest"]["scripts"];
	for(auto script:scripts){
		auto id=script["id"].as<std::string>();
		if(id.empty())continue;
		if(!check_arch(id,script))continue;
		log_info("found installer script {}",id);
		return script;
	}
	throw RuntimeError("no suitable installer script found in image manifest");
}

static std::vector<std::string>choose_shell(YAML::Node&script){
	if(auto v=script["shell"]){
		auto shell=v.as<std::string>();
		if(!shell.empty())return {path_find_exec(shell)};
	}
	try{return {path_find_exec("bash"),"-xe"};}catch(...){}
	if(auto sh=str_get_env("SHELL");!sh.empty())
		return {path_find_exec(sh)};
	try{return {path_find_exec("sh")};}catch(...){}
	throw RuntimeError("no available script shell");
}

static void start_script(YAML::Node&script){
	std::string exec{};
	std::vector<std::string>args{};
	std::map<std::string,std::string>envs{};
	auto id=script["id"].as<std::string>();
	auto type=script["type"].as<std::string>();
	auto path=script["path"].as<std::string>();
	if(type.empty())throw RuntimeError("script type not set");
	if(path.empty())throw RuntimeError("script path not set");
	if(type=="shell"){
		auto sh=choose_shell(script);
		exec=sh[0];
		std_add_list(args,sh);
	}else if(type=="execute"){
		exec=path_find_exec(path);
	}else throw RuntimeError("unsupported script type {} for script {}",type,id);
	args.push_back(path);
	envs=string_array_to_map((const char**)environ);
	if(auto vargs=script["args"];vargs.IsSequence())
		for(auto arg:vargs)if(arg.IsScalar())
			args.push_back(arg.as<std::string>());
	if(auto v=script["empty-env"];v.as<bool>(false))
		envs.clear();
	if(auto venvs=script["env"];venvs.IsMap())for(auto env:venvs)
		envs[env.first.as<std::string>()]=env.second.as<std::string>();
	std::string path_value{};
	path_value+=path_join(flasher.folder,"bin");
	if(envs.contains("PATH")&&!envs["PATH"].empty()){
		path_value+=':';
		path_value+=envs["PATH"];
	}
	envs["PATH"]=path_value;
	log_info("start script {}: {}",id,vector_to_string(args));
	flasher.script=std::make_shared<process>();
	flasher.script->exe=exec;
	flasher.script->args=args;
	flasher.script->envs=envs;
	flasher.script->set_fd(installer_status_fd,installer_status_fd);
	flasher.script->start();
	if(flasher.script->pid<0)
		throw RuntimeError("failed to start script {}",id);
	log_info("script {} started as pid {}",id,flasher.script->pid);
	flasher.pids->add_pid(flasher.script->pid);
}

static void wait_script(){
	if(!flasher.script||flasher.script->pid<0)
		throw RuntimeError("failed to start script");
	auto ret=flasher.script->wait();
	if(ret!=0)throw RuntimeError("script {} failed with {}",flasher.script->pid,ret);
	log_info("script exited successfully");
}

static void flasher_process(){
	choose_folder();
	prepare_image_mnt();
	load_image_manifest();
	flasher.pids=std::make_shared<cgroup_pids>("nanodistro-flasher");
	log_info("create cgroup {}",flasher.pids->get_path());
	auto script=choose_script();
	start_script(script);
	wait_script();
}

int flasher_do_main(){
	int ret=0;
	try{
		flasher_lock();
		chdir("/");
		flasher_process();
		log_info("flasher finished");
	}catch(const std::exception&exc){
		log_exception(exc,"flasher failed");
		ret=1;
	}
	flasher_cleanup();
	flasher_unlock();
	return ret;
}
