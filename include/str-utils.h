#ifndef STR_UTILS_H
#define STR_UTILS_H
#include<map>
#include<vector>
#include<string>
#include<memory>
#include<functional>
extern bool str_remove_end(std::string&str,const std::string&end);
extern bool str_remove_ends(std::string&str,const std::vector<std::string>&end);
extern void str_remove_all_end(std::string&str,const std::string&end);
extern void str_remove_all_ends(std::string&str,const std::vector<std::string>&end);
extern void str_trim_end(std::string&str);
extern void str_trim_start(std::string&str);
extern void str_trim(std::string&str);
extern bool str_contains(const std::string&str1,const std::string&str2);
extern std::string str_trim_end_to(const std::string&str);
extern std::string str_trim_start_to(const std::string&str);
extern std::string str_trim_to(const std::string&str);
extern std::vector<std::string>str_split(const std::string&str,const std::string&sep);
extern std::vector<std::string>str_split(const std::string&str,char sep);
extern std::map<std::string,std::string>parse_environ(const std::string&cont,char lsep='\n',char csep='=',bool comment=true);
extern std::multimap<std::string,std::string>parse_multi_environ(const std::string&cont,char lsep='\n',char csep='=',bool comment=true);
extern std::vector<std::string>string_array_to_vector(const char**arr,size_t count=0);
extern std::map<std::string,std::string>string_array_to_map(const char**arr,const std::string&sep="=");
extern std::string vector_to_string(const std::vector<std::string>&strs,const std::string&sep=" ");
extern std::unique_ptr<char*[],decltype(&free)>vector_to_string_array(const std::vector<std::string>&strs);
extern std::unique_ptr<char*[],decltype(&free)>map_to_string_array(const std::map<std::string,std::string>&strs,const std::string&sep="=");
extern std::string str_unicode_to_utf8(char32_t u);
extern std::string str_get_env(const std::string&str);
std::vector<std::string>parse_command(const std::string&cmd,std::function<std::string(const std::string&)>getvar=str_get_env);
extern std::string char_to_string(char c);
extern std::string vssprintf(const char*fmt,va_list va);
extern std::string ssprintf(const char*fmt,...);
extern int escape_len(char c);
extern std::string escape_parse(const std::string&esc);
extern bool check_ident_char(char c,bool first);
extern bool check_ident_string(const std::string&str);
extern int hex2dec(char hex);
extern char dec2hex(int dec,bool upper=false);
extern bool string_is_true(std::string str);
extern bool string_is_false(std::string str);
extern bool is_number(const std::string&str,int base=0);
extern void string_to_upper(std::string&str);
extern void string_to_lower(std::string&str);
#endif
