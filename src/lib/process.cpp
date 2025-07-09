#include"log.h"
#include"error.h"
#include"process.h"
#include"cleanup.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"std-utils.h"
#include"path-utils.h"
#include<cstring>
#include<unistd.h>
#include<sys/wait.h>

process::process(){
	envs=string_array_to_map((const char**)environ);
}

void process::set_fd(int id,int fd){
	if(id<0)return;
	fds[id]=fd<0?fd_desc::new_null():fd_desc::new_fd(fd);
}

void process::set_execute(const std::string&e){
	this->exe=path_find_exec(e);
}

void process::set_command(const std::vector<std::string>&cmd){
	if(cmd.empty())throw InvalidArgument("empty command");
	if(exe.empty())set_execute(cmd[0]);
	args=cmd;
}

void process::set_command(const std::string&cmd){
	set_command(parse_command(cmd));
}

void process::switch_to(){
	try{
		std::vector<int>skip{};
		for(auto&[_,desc]:fds)switch(desc.type){
			case fd_desc::FD:skip.push_back(desc.fd);break;
			case fd_desc::PIPE_IN:skip.push_back(desc.pipe[1]);break;
			case fd_desc::PIPE_OUT:skip.push_back(desc.pipe[0]);break;
			default:;
		}
		fs_close_all_fds(skip);
		for(auto&[dfd,desc]:fds){
			int fd;
			switch(desc.type){
				case fd_desc::FD:fd=desc.fd;break;
				case fd_desc::PIPE_IN:fd=desc.pipe[1];break;
				case fd_desc::PIPE_OUT:fd=desc.pipe[0];break;
				case fd_desc::PATH:
					fd=open(desc.path.c_str(),desc.flags,desc.mode);
					if(fd<0)throw ErrnoError(
						"desc {} open {} failed",
						dfd,desc.path
					);
				break;
				default:continue;
			}
			if(fd==dfd)continue;
			auto ret=dup2(fd,dfd);
			if(ret<0)throw ErrnoError("dup2 {} failed",dfd);
			close(fd);
		}
		auto argv=vector_to_string_array(args);
		auto envp=map_to_string_array(envs);
		if(!argv||!envp)_exit(1);
		if(standalone)setsid();
		if(uid!=-1&&getuid()!=uid&&setuid(uid)<0)
			throw ErrnoError("setuid {} failed",uid);
		if(gid!=-1&&getgid()!=gid&&setgid(gid)<0)
			throw ErrnoError("setgid {} failed",gid);
		if(euid!=-1&&geteuid()!=euid&&seteuid(euid)<0)
			throw ErrnoError("seteuid {} failed",euid);
		if(egid!=-1&&getegid()!=egid&&setegid(egid)<0)
			throw ErrnoError("setegid {} failed",egid);
		errno=0;
		execve(exe.c_str(),argv.get(),envp.get());
		log_error("failed to execute {}: {}",exe,strerror(errno));
	}catch(std::exception&exc){
		log_exception(exc,"switch to failed:");
	}
	_exit(1);
}

void process::start(){
	if(is_running())return;
	if(exe.empty())throw InvalidArgument("no exe set");
	if(args.empty())throw InvalidArgument("no arguments set");
	std::vector<int>clean{};
	cleanup_func clean_fn([clean]{
		for(auto fd:clean)close(fd);
	});
	auto create_pipe=[&](int pipes[2]){
		if(pipes[0]>=0)close(pipes[0]);
		if(pipes[1]>=0)close(pipes[1]);
		pipes[0]=-1;
		pipes[1]=-1;
		auto ret=pipe(pipes);
		if(ret<0)throw ErrnoError("pipe create failed");
		clean.push_back(pipes[0]);
		clean.push_back(pipes[1]);
	};
	for(auto&[_,desc]:fds){
		if(desc.type!=fd_desc::PIPE_IN&&desc.type!=fd_desc::PIPE_OUT)continue;
		create_pipe(desc.pipe);
	}
	auto pid=fork();
	if(pid<0)throw ErrnoError("fork failed");
	if(pid==0)switch_to();
	this->pid=pid;
	for(auto&[_,desc]:fds){
		if(desc.type==fd_desc::PIPE_IN&&desc.pipe[0]>=0)
			std_remove_item(clean,desc.pipe[0]);
		if(desc.type==fd_desc::PIPE_OUT&&desc.pipe[1]>=0)
			std_remove_item(clean,desc.pipe[1]);
	}
}

void process::kill(int sig){
	if(!is_running())return;
	auto ret=::kill(pid,sig);
	if(ret<0)throw ErrnoError("kill {} failed",pid);
}

void process::kill_wait(int sig,time_t timeout){
	if(!is_running())return;
	kill(sig);
	time_t start_time=time(nullptr);
	while(time(nullptr)-start_time<timeout){
		auto ret=waitpid(pid,nullptr,WNOHANG);
		if(ret==pid)return;
		if(ret==-1&&errno==ECHILD)return;
		usleep(100000);
	}
	log_info("wait pid {} exit timedout",pid);
	::kill(pid,SIGKILL);
	waitpid(pid,nullptr,0);
}

int process::wait(){
	int st=0,ret;
	if(!is_running())return exitcode;
	ret=waitpid(pid,&st,0);
	if(ret<0){
		if(errno==ECHILD)return exitcode;
		throw ErrnoError("waitpid {} failed",pid);
	}
	if(WIFEXITED(st))
		exitcode=WEXITSTATUS(st);
	else if(WIFSIGNALED(st))
		exitcode=128|WTERMSIG(st);
	return exitcode;
}

bool process::is_running(){
	if(pid<=0)return false;
	return fs_is_link(std::format("/proc/{}/exe",pid));
}
