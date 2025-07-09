#ifndef CGROUP_H
#define CGROUP_H
#include<vector>
#include<string>
#include<csignal>
class cgroup_pids{
	public:
		cgroup_pids(const std::string&name);
		~cgroup_pids();
		void add_pid(pid_t pid);
		void remove_pid(pid_t pid);
		void set_limit(int limit);
		std::vector<pid_t>list_pids();
		void killall(int sig=SIGTERM,int timeout=10,int killsig=SIGKILL);
		std::string get_path()const;
	public:
		std::string get_proc_path()const;
		std::string ctrl{};
		std::string name{};
};
#endif
