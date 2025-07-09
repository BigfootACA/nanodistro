#include<atomic>
#include"log.h"
#include"error.h"
#include"status.h"
#include"str-utils.h"

struct logging_context{
	~logging_context();
	std::atomic<uint64_t>hook_id=0;
	std::map<uint64_t,log_print_hook>print_hooks{};
	std::map<uint64_t,log_print_string_hook>print_string_hooks{};
	std::mutex print_mutex{};
};

static bool ctx_valid=true;
static log_level level_default=LOG_INFO;
static logging_context log_ctx{};

logging_context::~logging_context(){
	ctx_valid=false;
}

int log_init(){
	if(auto level=getenv("LOGLEVEL")){
		level_default=(log_level)std::stoi(level);
		if(level_default<LOG_ERROR)
			level_default=LOG_ERROR;
		if(level_default>LOG_TRACE)
			level_default=LOG_TRACE;
	}
	return 0;
}

static bool log_pipe_hook(log_level level,const std::string&msg,const log_location&loc){
	if(installer_status_fd<0)return true;
	Json::FastWriter writer;
	Json::Value json;
	log_tojson(json,level,msg,loc);
	auto s=writer.write(json);
	if(s.empty())return true;
	str_trim(s);
	if(s.find('\n')!=std::string::npos)return true;
	installer_write_cmd(std::format("log {}\n",s));
	return false;
}

std::string log_level_to_string(log_level level){
	switch(level){
		case LOG_ERROR:   return "ERROR";
		case LOG_WARNING: return "WARNING";
		case LOG_INFO:    return "INFO";
		case LOG_DEBUG:   return "DEBUG";
		case LOG_TRACE:   return "TRACE";
		default:          return "UNKNOWN";
	}
}

log_level log_level_from_string(const std::string&level){
	if(level=="ERROR")return LOG_ERROR;
	if(level=="WARNING")return LOG_WARNING;
	if(level=="INFO")return LOG_INFO;
	if(level=="DEBUG")return LOG_DEBUG;
	if(level=="TRACE")return LOG_TRACE;
	return level_default;
}

void log_print(log_level level,const std::string&msg,const log_location&loc){
	if(level>level_default)return;
	std::lock_guard<std::mutex>lock(log_ctx.print_mutex);
	if(installer_status_fd>=0&&!log_pipe_hook(level,msg,loc))return;
	if(ctx_valid)for(auto&[id,func]:log_ctx.print_hooks)try{
		if(!func(level,msg,loc))return;
	}catch(...){}
	auto log=std::format(
		"[{}]{}: {}",
		log_level_to_string(level),
		loc.tostring(),
		str_trim_to(msg)
	);
	if(ctx_valid)for(auto&[id,func]:log_ctx.print_string_hooks)try{
		if(!func(log))return;
	}catch(...){}
	fprintf(stderr,"%s\r\n",log.c_str());
}

void log_print(log_level level,const std::string&msg,std::source_location loc){
	if(level>level_default)return;
	log_location l(loc);
	log_print(level,msg,l);
}

void log_exception_(const std::exception_ptr&exc,const std::string&msg){
	try{
		if(exc)std::rethrow_exception(exc);
	}catch(std::exception&re){
		log_exception_(re,msg);
		return;
	}catch(...){}
	log_print(LOG_ERROR,msg,log_location("exception"));
}

void log_exception_(const std::exception&exc,const std::string&msg){
	std::string xmsg=msg;
	log_location loc{};
	loc.file="exception";
	if(auto re=dynamic_cast<const RuntimeErrorImpl*>(&exc)){
		if(!re->msg.empty())
			xmsg+=": "+re->msg;
		loc=re->loc;
	}else if(auto v=exc.what())
		xmsg+=": "+std::string(v);
	log_print(LOG_ERROR,xmsg,loc);
}

void log_assert_failed(const char*msg){
	log_print(LOG_ERROR,msg);
	abort();
}

log_location::log_location(const std::string&func):func(func){}

log_location::log_location(const std::source_location&loc){
	file=loc.file_name();
	func=loc.function_name();
	line=loc.line();
	column=loc.column();
}

static std::string simpify_source_path(const std::string&path){
	auto loc=std::source_location::current();
	const std::string curfile=loc.file_name();
	static constexpr std::string self="lib/logging.cpp";
	auto cpos=curfile.find(self);
	if(cpos==std::string::npos)return path;
	auto prefix=curfile.substr(0,cpos);
	if(!path.starts_with(prefix))return path;
	return path.substr(prefix.length());
}

std::string log_location::tostring()const{
	std::string s{};
	if(!file.empty())s+=simpify_source_path(file);
	if(line>0){
		if(!s.empty())s+=":";
		s+=std::to_string(line);
	}
	if(s.empty())s="unknown";
	return s;
}

uint64_t log_add_print_hook(const log_print_hook&hook){
	if(!hook)return 0;
	auto id=++log_ctx.hook_id;
	log_ctx.print_hooks[id]=hook;
	return id;
}

void log_del_print_hook(uint64_t id){
	if(!ctx_valid)return;
	if(log_ctx.print_hooks.find(id)==log_ctx.print_hooks.end())return;
	log_ctx.print_hooks.erase(id);
}

uint64_t log_add_print_string_hook(const log_print_string_hook&hook){
	if(!ctx_valid||!hook)return 0;
	auto id=++log_ctx.hook_id;
	log_ctx.print_string_hooks[id]=hook;
	return id;
}

void log_del_print_string_hook(uint64_t id){
	if(!ctx_valid)return;
	if(log_ctx.print_string_hooks.find(id)==log_ctx.print_string_hooks.end())return;
	log_ctx.print_string_hooks.erase(id);
}

void log_print_json(const Json::Value&json){
	log_location loc{};
	auto s=json["msg"].asString();
	if(s.empty())return;
	if(json.isMember("location"))
		loc.fromjson(json["location"]);
	auto level=log_level_from_string(json["level"].asString());
	log_print(level,s,loc);
}

Json::Value log_location::tojson()const{
	Json::Value json;
	json["file"]=file;
	json["func"]=func;
	json["line"]=line;
	json["column"]=column;
	return json;
}

void log_location::fromjson(const Json::Value&json){
	if(json.isMember("file"))
		file=json["file"].asString();
	if(json.isMember("func"))
		func=json["func"].asString();
	if(json.isMember("line"))
		line=json["line"].asUInt();
	if(json.isMember("column"))
		column=json["column"].asUInt();
}

void log_tojson(
	Json::Value&json,
	log_level level,
	const std::string&msg,
	const log_location&loc
){
	json["level"]=log_level_to_string(level);
	json["msg"]=msg;
	json["location"]=loc.tojson();
}
