#include<atomic>
#include<cstring>
#include<csignal>
#include"internal.h"
#include"fs-utils.h"
#include"status.h"
#include"error.h"
#include"main.h"
#include"log.h"

static std::shared_ptr<netblk_implement>impl=nullptr;

static int usage(int e){
	fprintf(e==0?stdout:stderr,"Usage: netblk IN_JSON OUT_JSON\n");
	return e;
}

static void signal_handler(int sig){
	if(sig==SIGUSR2){
		psi_monitor::get()->want_gc=true;
		return;
	}
	if(sig==SIGUSR1||(impl&&!impl->running))return;
	log_info("netblk received exit signal {}",sig);
	if(impl)impl->running=false;
}

static void install_signal(){
	struct sigaction sa{};
	sa.sa_handler=signal_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT,&sa,nullptr);
	sigaction(SIGTERM,&sa,nullptr);
	sigaction(SIGQUIT,&sa,nullptr);
	sigaction(SIGUSR1,&sa,nullptr);
	sigaction(SIGUSR2,&sa,nullptr);
	signal(SIGHUP,SIG_IGN);
	signal(SIGPIPE,SIG_IGN);
}

static int post_main(int argc,char**argv){
	install_signal();
	if(auto r=installer_init())return r;
	if(auto r=log_init())return r;
	if(auto r=locale_init())return r;
	if(argc!=3)return usage(1);
	auto inf=std::string(argv[1]);
	auto outf=std::string(argv[2]);
	Json::Reader reader{};
	Json::StyledWriter writer{};
	Json::Value inj{},outj{};
	auto ins=fs_read_all(inf);
	if(!reader.parse(ins,inj))throw RuntimeError(
		"failed to parse input json {}: {}",
		inf,reader.getFormattedErrorMessages()
	);
	auto backend=netblk_backend::create("",inj["backend"]);
	impl=netblk_implement::choose(inj["device"]);
	auto dev=impl->create(backend);
	outj=inj;
	outj["backend"]["driver"]=backend->get_name();
	outj["backend"]["size"]=backend->get_size();
	outj["device"]["driver"]=impl->get_name();
	outj["device"]["path"]=dev->get_path();
	auto outs=writer.write(outj);
	fs_write_all(outf,outs);
	log_info("output json wrote to {}",outf);
	impl->loop();
	log_info("exiting");
	return 0;
}

int netblk_main(int argc,char**argv){
	try{
		return post_main(argc,argv);
	}catch(std::exception&exc){
		log_exception(exc,"fatal exception in main");
	}
	return 1;
}
