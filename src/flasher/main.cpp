#include"main.h"
#include"log.h"
#include"internal.h"
#include"status.h"
#include"fs-utils.h"

static int flasher_usage(int e){
	fprintf(e==0?stdout:stderr,"Usage: flasher JSON\n");
	return e;
}

static void exit_signal(int sig){
	log_info("flasher received exit signal {}",sig);
	if(flasher.script->pid>0)
		kill(flasher.script->pid,SIGTERM);
}

static void install_signal(){
	struct sigaction sa{};
	sa.sa_handler=exit_signal;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT,&sa,nullptr);
	sigaction(SIGTERM,&sa,nullptr);
	sigaction(SIGQUIT,&sa,nullptr);
	sigaction(SIGHUP,&sa,nullptr);
	sigaction(SIGPIPE,&sa,nullptr);
}

static int post_main(int argc,char**argv){
	int r;
	if((r=installer_init())!=0)return r;
	if((r=log_init())!=0)return r;
	if((r=locale_init())!=0)return r;
	if(argc!=2)return flasher_usage(1);
	if(!fs_is_file(argv[1]))return flasher_usage(1);
	install_signal();
	installer_load_context(argv[1]);
	log_info("flasher started");
	return flasher_do_main();
}

int flasher_main(int argc,char**argv){
	try{
		return post_main(argc,argv);
	}catch(const std::exception&exc){
		log_exception(exc,"fatal exception in main");
		return 1;
	}
}
