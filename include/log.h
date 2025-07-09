#ifndef LOG_H
#define LOG_H
#include<format>
#include<source_location>
#include<libintl.h>
#include<json/json.h>
#include<functional>

#define _(str) gettext(str)

enum log_level{
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
	LOG_TRACE,
};

struct log_location{
	std::string file{};
	std::string func{};
	uint32_t line=0;
	uint32_t column=0;
	inline log_location(){}
	log_location(const std::string&func);
	log_location(const std::source_location&loc);
	std::string tostring()const;
	Json::Value tojson()const;
	void fromjson(const Json::Value&json);
};

using log_print_hook=std::function<bool(log_level,const std::string&,const log_location&)>;
using log_print_string_hook=std::function<bool(const std::string&)>;

extern int log_init();
extern log_level log_level_from_string(const std::string&level);
extern std::string log_level_to_string(log_level level);
extern uint64_t log_add_print_hook(const log_print_hook&hook);
extern uint64_t log_add_print_string_hook(const log_print_string_hook&hook);
extern void log_del_print_hook(uint64_t id);
extern void log_del_print_string_hook(uint64_t id);
extern void log_print_json(const Json::Value&json);
extern void log_print(log_level level,const std::string&msg,const log_location&loc);
extern void log_print(log_level level,const std::string&msg,std::source_location loc=std::source_location::current());
extern void log_exception_(const std::exception&exc,const std::string&msg);
extern void log_exception_(const std::exception_ptr&exc,const std::string&msg);
extern void log_tojson(Json::Value&json,log_level level,const std::string&msg,const log_location&loc);
#define log_error(...) log_print(LOG_ERROR,std::format(__VA_ARGS__))
#define log_warning(...) log_print(LOG_WARNING,std::format(__VA_ARGS__))
#define log_info(...) log_print(LOG_INFO,std::format(__VA_ARGS__))
#define log_debug(...) log_print(LOG_DEBUG,std::format(__VA_ARGS__))
#define log_trace(...) log_print(LOG_TRACE,std::format(__VA_ARGS__))
#define log_exception(exc,...) log_exception_(exc,std::format(__VA_ARGS__))
#define log_cur_exception(...) log_exception(std::current_exception(),__VA_ARGS__)
extern "C"{
	extern void log_assert_failed(const char*msg);
}
#endif
