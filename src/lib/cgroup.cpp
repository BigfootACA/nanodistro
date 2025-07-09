#include<unistd.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<libmount/libmount.h>
#include"cgroup.h"
#include"cleanup.h"
#include"error.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"path-utils.h"
#include"libmounts.h"

std::string cgroup_find_pid_ctrl(){
	int ret;
	auto tab=mnt_new_table();
	if(!tab)throw RuntimeError("failed to allocate mount table");
	cleanup_func tab_cleanup(std::bind(mnt_free_table,tab));
	if((ret=mnt_table_parse_mtab(tab,nullptr))!=0)
		throw RuntimeError("failed to parse mountinfo: {}",libmount_strerror(ret));
	auto itr=mnt_new_iter(MNT_ITER_FORWARD);
	if(!itr)throw RuntimeError("failed to allocate mount iterator");
	cleanup_func itr_cleanup(std::bind(mnt_free_iter,itr));
	libmnt_fs*fs=nullptr;
	mnt_reset_iter(itr,MNT_ITER_FORWARD);
	while(mnt_table_next_fs(tab,itr,&fs)==0){
		auto target=mnt_fs_get_target(fs);
		auto fstype=mnt_fs_get_fstype(fs);
		if(!target||std::string(target)!="/sys/fs/cgroup")continue;
		if(!fstype||std::string(fstype)!="cgroup2")continue;
		return target;
	}
	mnt_reset_iter(itr,MNT_ITER_FORWARD);
	while(mnt_table_next_fs(tab,itr,&fs)==0){
		auto fstype=mnt_fs_get_fstype(fs);
		auto opts=mnt_fs_get_options(fs);
		auto target=mnt_fs_get_target(fs);
		if(!fstype||std::string(fstype)!="cgroup")continue;
		if(!opts||!str_contains(opts,"pids"))continue;
		if(target)return target;
	}
	mnt_reset_iter(itr,MNT_ITER_FORWARD);
	while(mnt_table_next_fs(tab,itr,&fs)==0){
		auto target=mnt_fs_get_target(fs);
		auto fstype=mnt_fs_get_fstype(fs);
		if(!fstype||std::string(fstype)!="cgroup2")continue;
		if(target)return target;
	}
	throw RuntimeError("no pids cgroup found");
}

cgroup_pids::cgroup_pids(const std::string&name):name(name){
	ctrl=cgroup_find_pid_ctrl();
	auto path=get_path();
	rmdir(path.c_str());
	auto ret=mkdir(path.c_str(),0755);
	if(ret!=0)throw ErrnoError("failed to create cgroup {}",path);
}

cgroup_pids::~cgroup_pids(){
	auto ret=rmdir(get_path().c_str());
	if(ret!=0&&errno!=ENOENT)log_warning(
		"failed to remove cgroup {}: {}",
		get_path(),strerror(errno)
	);
}

void cgroup_pids::add_pid(pid_t pid){
	fs_append_numlf(get_proc_path(),pid);
}

void cgroup_pids::remove_pid(pid_t pid){
	auto lst=list_pids();
	lst.erase(std::remove(lst.begin(),lst.end(),pid),lst.end());
	std::string data={};
	for(auto&pid:lst)data+=std::to_string(pid),data+='\n';
	fs_write_all(get_proc_path(),data);
}

void cgroup_pids::set_limit(int limit){
	auto path=path_join(get_path(),"pids.max");
	fs_write_numlf(path,limit);
}

std::vector<pid_t>cgroup_pids::list_pids(){
	std::vector<pid_t>ret{};
	auto path=get_proc_path();
	try{
		auto data=fs_read_all(path);
		for(auto&line:str_split(data,'\n')){
			if(line.empty())continue;
			size_t idx=0;
			auto pid=std::stoi(line,&idx);
			if(idx!=line.length())continue;
			ret.push_back(pid);
		}
	}catch(std::exception&exc){
		log_exception(exc,"failed to read cgroup {}",path);
	}
	return ret;
}

void cgroup_pids::killall(int sig,int timeout,int killsig){
	auto start=time(nullptr);
	int cursig=sig;
	while(true){
		auto now=time(nullptr);
		auto lst=list_pids();
		if(lst.empty())break;
		if(timeout>0&&now-start>timeout){
			if(killsig==0||cursig==killsig)
				throw RuntimeError("timeout waiting for cgroup {} to exit",name);
			log_warning("timeout waiting for cgroup {} to exit",name);
			cursig=killsig,start=now;
		}
		for(auto&pid:lst)kill(pid,cursig);
		for(auto&pid:lst)waitpid(pid,nullptr,WNOHANG);
		if(list_pids().empty())break;
		usleep(200000);
	}
}

std::string cgroup_pids::get_path()const{
	return path_join(ctrl,name);
}

std::string cgroup_pids::get_proc_path()const{
	return path_join(get_path(),"cgroup.procs");
}
