#include"log.h"
#include"configs.h"
#include"str-utils.h"
#include<sys/syslog.h>

#ifdef LOG_ERR
#undef LOG_ERR
#endif
#ifdef LOG_WARNING
#undef LOG_WARNING
#endif
#ifdef LOG_INFO
#undef LOG_INFO
#endif
#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif
#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif

static bool log_syslog_hook(log_level level,const std::string&msg,const log_location&loc){
	std::string s=msg;
	str_trim(s);
	if(s.empty())return true;
	int prio=LOG_INFO;
	switch(level){
		case log_level::LOG_ERROR:   prio=3;break;
		case log_level::LOG_WARNING: prio=4;break;
		case log_level::LOG_INFO:    prio=6;break;
		case log_level::LOG_DEBUG:   prio=7;break;
		case log_level::LOG_TRACE:   prio=7;break;
		default:prio=6;break;
	}
	syslog(prio,"%s: %s",loc.tostring().c_str(),s.c_str());
	return true;
}

int log_init_syslog(){
	bool use_syslog=false;
	if(auto v=config["nanodistro"]["syslog"])
		use_syslog=v.as<bool>();
	if(!use_syslog)return 0;
	openlog("nanodistro",LOG_PID|LOG_NDELAY,LOG_USER);
	log_add_print_hook(log_syslog_hook);
	return 0;
}
