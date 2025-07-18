#include<csignal>
#include<lvgl.h>
#include<getopt.h>
#include<unistd.h>
#include<sys/time.h>
#include"log.h"
#include"gui.h"
#include"main.h"
#include"configs.h"
#include"fs-utils.h"
#include"time-utils.h"
#include"error.h"

std::atomic<bool>gui_running=false;
std::atomic<bool>gui_pause=false;

static log_level lvgl_log_level_to_log_level(lv_log_level_t level){
	switch(level){
		case LV_LOG_LEVEL_TRACE:   return LOG_TRACE;
		case LV_LOG_LEVEL_INFO:    return LOG_INFO;
		case LV_LOG_LEVEL_WARN:    return LOG_WARNING;
		case LV_LOG_LEVEL_ERROR:   return LOG_ERROR;
		case LV_LOG_LEVEL_USER:    return LOG_INFO;
		default:                   return LOG_DEBUG;
	}
}

static int usage(int e){
	fprintf(e==0?stdout:stderr,
		"Usage: nanodistro [options]\n"
		"Options:\n"
		"  -c, --config <CFG>  Set config path\n"
		"  -h, --help          Show this help message\n"
	);
	return e;
}

static const option lopts[]={
	{"config", required_argument, NULL, 'c'},
	{"help",   no_argument,       NULL, 'h'},
	{NULL,0,NULL,0}
};
static const char*sopts="c:h";

static int load_config(int argc,char**argv){
	int o;
	const char*cfg=nullptr;
	try{
		while((o=getopt_long(argc,argv,sopts,lopts,NULL))!=-1)switch(o){
			case 'c':
				if(cfg)throw InvalidArgument("multiple config options");
				cfg=optarg;
			break;
			case 'h':return usage(0);
			default:throw InvalidArgument("unknown option {:c}",o);
		}
		if(optind<argc)throw InvalidArgument("too many arguments");
	}catch(std::exception&exc){
		log_exception(exc,"failed to parse options");
		return usage(1);
	}
	if(!cfg)cfg="/etc/nanodistro/config.yaml";

	if(!fs_exists(cfg))
		throw InvalidArgument("config file {} not found",cfg);

	try{
		log_info("loading config file {}",cfg);
		config=YAML::LoadFile(cfg);
	}catch(std::exception&exc){
		log_exception(exc,"failed to parse config file {}",cfg);
		return 1;
	}
	return 0;
}

int locale_init(){
	setlocale(LC_ALL,"");
	std::string locale_dir{};
	if(auto v=config["nanodistro"]["locale-dir"])
		locale_dir=v.as<std::string>();
	bindtextdomain("nanodistro",locale_dir.empty()?nullptr:locale_dir.c_str());
	textdomain("nanodistro");
	return 0;
}

static void log_quirk_filter(std::string&msg,log_level&level){
	if(msg.ends_with(": begin"))level=LOG_TRACE;
	if(msg.starts_with("grid_update: update 0x"))level=LOG_TRACE;
	if(msg.starts_with("indev_keypad_proc: "))level=LOG_TRACE;
}

static void lvgl_logger(lv_log_level_t level,const char*msg){
	static const std::vector<std::string>lvl_prefix{
		"Trace","Info","Warn","Error","User"
	};
	std::string xmsg=msg;
	log_location loc{};
	loc.file="lvgl";
	auto l=lvgl_log_level_to_log_level(level);
	#if LV_LOG_USE_FILE_LINE
	if(auto npos=xmsg.rfind(' ');npos!=std::string::npos){
		auto lpos=xmsg.find(':',npos+1);
		auto cpos=xmsg.find('.',npos+1);
		if(lpos!=std::string::npos&&cpos!=std::string::npos){
			loc.file=xmsg.substr(npos+1,lpos-npos-1);
			loc.line=std::stoi(xmsg.substr(lpos+1));
			xmsg.erase(npos,std::string::npos);
		}
	}
	#endif	
	auto tag=std::format("[{}] ",lvl_prefix[level]);
	if(xmsg.starts_with(tag))xmsg.erase(0,tag.length());
	log_quirk_filter(xmsg,l);
	log_print(l,xmsg,loc);
}

static int lvgl_init(){
	lv_init();
	lv_log_register_print_cb(lvgl_logger);
	lv_tick_set_cb([]{
		return (uint32_t)timestamp::boottime().tomillisecond();
	});
	lv_delay_set_cb([](auto d){
		timestamp(d,timestamp::millisecond).sleep();
	});
	return 0;
}
 
static void signal_handler(int sig){
	if(sig==SIGUSR1||sig==SIGUSR2){
		display_handle_signal(sig);
		return;
	}
	if(!gui_running)return;
	log_info("nanodistro received exit signal {}",sig);
	gui_running=false;
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

static int lvgl_main(){
	log_info("main loop started");
	auto last_tick=timestamp::boottime();
	gui_running=true;
	while(gui_running){
		auto current=timestamp::boottime();
		auto elapsed=current-last_tick;
		lv_tick_inc(elapsed.tomillisecond());
		last_tick=current;
		auto next=lv_timer_handler();
		if(next==LV_NO_TIMER_READY)next=LV_DEF_REFR_PERIOD;
		if(next>0)timestamp(next,timestamp::millisecond).sleepi();
		while(gui_running&&gui_pause)
			timestamp(250,timestamp::millisecond).sleepi();
	}
	return 0;
}

static int post_main(int argc,char**argv){
	unsetenv("NANODISTRO_STATUS_FD");
	install_signal();
	if(auto r=log_init())return r;
	if(auto r=load_config(argc,argv))return r;
	if(auto r=log_init_syslog())return r;
	if(auto r=locale_init())return r;
	if(auto r=lvgl_init())return r;
	if(auto r=display_init())return r;
	if(auto r=input_init())return r;
	if(auto r=image_init())return r;
	if(auto r=fonts_init())return r;
	if(auto r=gui_init())return r;
	return lvgl_main();
}

int nanodistro_main(int argc,char**argv){
	try{
		return post_main(argc,argv);
	}catch(std::exception&exc){
		log_exception(exc,"fatal exception in main");
		return 1;
	}
}
