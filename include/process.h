#ifndef PROCESS_H
#define PROCESS_H
#include<map>
#include<vector>
#include<string>
#include<memory>
#include<fcntl.h>
#include<signal.h>
struct fd_desc{
	enum fd_type{
		FD,
		PATH,
		PIPE_IN,
		PIPE_OUT,
	}type;
	int fd=-1,flags=0;
	mode_t mode=0;
	int pipe[2]{-1,-1};
	std::string path{};
	static inline fd_desc new_fd(int fd){return {FD,fd};}
	static inline fd_desc new_pipe_in(int pipe[2]){return {PIPE_IN,-1,0,0,{pipe[0],pipe[1]}};}
	static inline fd_desc new_pipe_out(int pipe[2]){return {PIPE_OUT,-1,0,0,{pipe[0],pipe[1]}};}
	static inline fd_desc new_path(const std::string&path,int flags=O_RDWR,mode_t mode=0644){
		return {PATH,-1,flags,mode,{-1,-1},path};
	}
	static inline fd_desc new_null(){return new_path("/dev/null",O_RDWR|O_NOCTTY);}
};
class process:public std::enable_shared_from_this<process>{
	public:
		process();
		uid_t uid=-1,euid=-1;
		gid_t gid=-1,egid=-1;
		int exitcode=0;
		pid_t pid=-1;
		bool standalone=false;
		std::map<int,fd_desc>fds={
			{0,{fd_desc::FD,0}},
			{1,{fd_desc::FD,1}},
			{2,{fd_desc::FD,2}},
		};
		std::string exe{};
		std::vector<std::string>args{};
		std::map<std::string,std::string>envs{};
		void set_fd(int id,int fd);
		void set_execute(const std::string&exe);
		void set_command(const std::string&cmd);
		void set_command(const std::vector<std::string>&cmd);
		void start();
		bool is_running();
		void kill(int sig=SIGTERM);
		void kill_wait(int sig=SIGTERM,time_t timeout=10);
		int wait();
	private:
		void switch_to();
};
#endif
