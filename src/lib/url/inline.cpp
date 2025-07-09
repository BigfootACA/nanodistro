#include "url.h"

url::url(const char*u,size_t len){parse(u,len);}
url::url(const char*u){parse(u);}
url::url(const std::string&str){parse(str);}
url::url(const std::string*str){parse(str);}
url::url(const url&str){from(str);}
url::url(const url*str){from(str);}
void url::parse(const std::string*u){parse(u->c_str(),u->length());}
void url::parse(const std::string&u){parse(u.c_str(),u.length());}
void url::parse(const char*u){parse(u,strlen(u));}
void url::from(const url&url){from(&url);}
std::string url::get_origin()const{
	std::string out;
	append_origin(out);
	return out;
}
std::string url::get_hierarchical()const{
	std::string out;
	append_hierarchical(out);
	return out;
}
std::string url::get_user_info()const{
	std::string out;
	append_user_info(out);
	return out;
}
std::string url::get_authority()const{
	std::string out;
	append_authority(out);
	return out;
}
std::string url::get_full_path()const{
	std::string out;
	append_full_path(out);
	return out;
}
std::string url::get_url()const{
	std::string out;
	append_all(out);
	return out;
}
bool url::is_in_top()const{return path.empty()||path=="/";}
std::string url::to_string()const{return get_url();}
std::string url::get_scheme()const{return scheme;}
std::string url::get_username()const{return username;}
std::string url::get_password()const{return password;}
std::string url::get_host()const{return host;}
int url::get_port()const{return port;}
std::string url::get_path()const{return path;}
std::string url::get_query()const{return query;}
std::string url::get_fragment()const{return fragment;}
void url::set_scheme(const std::string&val){scheme=val;}
void url::set_username_decoded(const std::string&val){username=val;}
void url::set_password_decoded(const std::string&val){password=val;}
void url::set_host_decoded(const std::string&val){host=val;}
void url::set_port(int val){port=val;}
void url::set_path_decoded(const std::string&val){path=val;}
void url::set_query(const std::string&val){query=val;}
void url::set_fragment(const std::string&val){fragment=val;}
void url::set_username(const std::string&val){
	set_username_decoded(decode(val));
}
void url::set_password(const std::string&val){
	set_password_decoded(decode(val));
}
void url::set_host(const std::string&val){
	set_host_decoded(decode(val));
}
void url::set_path(const std::string&val){
	set_path_decoded(decode(val));
}
void url::set_scheme(const char*val){
	set_scheme(std::string(val));
}
void url::set_username(const char*val){
	set_username(std::string(val));
}
void url::set_password(const char*val){
	set_password(std::string(val));
}
void url::set_host(const char*val){
	set_host(std::string(val));
}
void url::set_port(const char*val){
	set_port(std::string(val));
}
void url::set_path(const char*val){
	set_path(std::string(val));
}
void url::set_query(const char*val){
	set_query(std::string(val));
}
void url::set_fragment(const char*val){
	set_fragment(std::string(val));
}
void url::set_username_decoded(const char*val){
	set_username_decoded(std::string(val));
}
void url::set_password_decoded(const char*val){
	set_password_decoded(std::string(val));
}
void url::set_host_decoded(const char*val){
	set_host_decoded(std::string(val));
}
void url::set_path_decoded(const char*val){
	set_path_decoded(std::string(val));
}
void url::set_scheme(const char*val,size_t len){
	set_scheme(std::string(val,len?len:strlen(val)));
}
void url::set_username(const char*val,size_t len){
	set_username(std::string(val,len?len:strlen(val)));
}
void url::set_password(const char*val,size_t len){
	set_password(std::string(val,len?len:strlen(val)));
}
void url::set_host(const char*val,size_t len){
	set_host(std::string(val,len?len:strlen(val)));
}
void url::set_port(const char*val,size_t len){
	set_port(std::string(val,len?len:strlen(val)));
}
void url::set_path(const char*val,size_t len){
	set_path(std::string(val,len?len:strlen(val)));
}
void url::set_query(const char*val,size_t len){
	set_query(std::string(val,len?len:strlen(val)));
}
void url::set_fragment(const char*val,size_t len){
	set_fragment(std::string(val,len?len:strlen(val)));
}
void url::set_username_decoded(const char*val,size_t len){
	set_username_decoded(std::string(val,len?len:strlen(val)));
}
void url::set_password_decoded(const char*val,size_t len){
	set_password_decoded(std::string(val,len?len:strlen(val)));
}
void url::set_host_decoded(const char*val,size_t len){
	set_host_decoded(std::string(val,len?len:strlen(val)));
}
void url::set_path_decoded(const char*val,size_t len){
	set_path_decoded(std::string(val,len?len:strlen(val)));
}
int url::compare(const url&u)const{return compare(&u);}
int url::compare(const char*str)const{return compare(std::string(str));}
int url::compare(const std::string*u)const{return compare(*u);}
bool url::equals(const char*str)const{return compare(str)==0;}
bool url::equals(const std::string*u)const{return compare(u)==0;}
bool url::equals(const std::string&u)const{return compare(u)==0;}
bool url::equals(const url&u)const{return compare(u)==0;}
bool url::equals(const url*u)const{return compare(u)==0;}
url&url::operator=(const char*u){parse(u);return *this;}
url&url::operator=(std::string*u){parse(u);return *this;}
url&url::operator=(std::string&u){parse(u);return *this;}
url&url::operator=(const url*u){from(u);return *this;}
url&url::operator=(const url&u){from(u);return *this;}
bool url::operator==(const char*u)const{return this->equals(u);}
bool url::operator==(std::string*u)const{return this->equals(u);}
bool url::operator==(std::string&u)const{return this->equals(u);}
bool url::operator==(const url*u)const{return this->equals(u);}
bool url::operator==(const url&u)const{return this->equals(u);}
bool url::operator!=(const char*u)const{return !this->equals(u);}
bool url::operator!=(std::string*u)const{return !this->equals(u);}
bool url::operator!=(std::string&u)const{return !this->equals(u);}
bool url::operator!=(const url*u)const{return !this->equals(u);}
bool url::operator!=(const url&u)const{return !this->equals(u);}
bool url::operator>(const url*u)const{return this->compare(u)>0;}
bool url::operator>(const url&u)const{return this->compare(u)>0;}
bool url::operator>=(const url*u)const{return this->compare(u)>=0;}
bool url::operator>=(const url&u)const{return this->compare(u)>=0;}
bool url::operator<(const url*u)const{return this->compare(u)<0;}
bool url::operator<(const url&u)const{return this->compare(u)<0;}
bool url::operator<=(const url*u)const{return this->compare(u)<=0;}
bool url::operator<=(const url&u)const{return this->compare(u)<=0;}
std::ostream&url::operator<<(std::ostream&os)const{os<<to_string();return os;}
url::operator std::string()const{return to_string();}
