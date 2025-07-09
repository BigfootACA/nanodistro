#include<poll.h>
#include<sys/wait.h>
#include<sys/prctl.h>
#include"internal.h"
#include"str-utils.h"
#include"log.h"

void ui_draw_progress::pty_dispatch_thread(std::stop_token stop){
	int st=0;
	prctl(PR_SET_NAME,"pty-dispatch");
	while(!stop.stop_requested()){
		pollfd pfd[2]{};
		int cnt=0;
		if(pty_bridge>=0){
			pfd[cnt].fd=pty_bridge;
			pfd[cnt].events=POLLIN;
			cnt++;
		}
		if(status_fd[0]>=0){
			pfd[cnt].fd=status_fd[0];
			pfd[cnt].events=POLLIN;
			cnt++;
		}
		if(pty_pid>0&&waitpid(pty_pid,&st,WNOHANG)==pty_pid){
			std::string str{},log{};
			bool success=false;
			if(WIFSIGNALED(st)){
				auto sig=WTERMSIG(st);
				str=ssprintf(_("[Process %d exited by signal %d]"),pty_pid,sig);
				log=std::format("process {} exited by signal {}",pty_pid,sig);
			}else if(WIFEXITED(st)){
				auto code=WEXITSTATUS(st);
				success=code==0;
				str=ssprintf(_("[Process %d exited with status %d]"),pty_pid,code);
				log=std::format("process {} exited with status {}",pty_pid,code);
			}else{
				str=ssprintf(_("[Process %d exited]"),pty_pid);
				log=std::format("process {} exited",pty_pid);
			}
			proc_status();
			log_info("{}",log);
			lv_thread_call_func(std::bind(&lv_termview_line_prints,termview,str));
			lv_thread_call_func(std::bind(&ui_draw_progress::on_exited,this,success));
			pty_pid=-1;
			thread.request_stop();
			continue;
		}
		if(poll(pfd,cnt,1000)<0)switch(errno){
			case EAGAIN:usleep(50000);//fallthrough
			case EINTR:continue;
			default:
				log_warning("poll failed: {}",strerror(errno));
				proc_status();
				auto str=ssprintf(_("[poll failed: %s]"),_(strerror(errno)));
				lv_thread_call_func(std::bind(&lv_termview_line_prints,termview,str));
				lv_thread_call_func(std::bind(&ui_draw_progress::on_exited,this,false));
				thread.request_stop();
				continue;
		}
		for(int i=0;i<cnt;i++){
			if(pfd[i].revents==0)continue;
			if(pfd[i].fd==pty_bridge){
				auto f=std::bind(&ui_draw_progress::pty_dispatch_task,this);
				lv_thread_call_func(f);
				sem.acquire();
			}else if(pfd[i].fd==status_fd[0]){
				proc_status();
			}
		}
	}
}
