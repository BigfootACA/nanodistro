#include<string>
#include<cctype>
#include<cstdarg>
#include<cstring>
#include"error.h"
#include"str-utils.h"

bool str_remove_end(std::string&str,const std::string&end){
	if(end.empty())return false;
	auto ret=str.ends_with(end);
	if(ret){
		auto d=str.end();
		auto len=end.length();
		str.erase(d-(ptrdiff_t)len,d);
	}
	return ret;
}

bool str_remove_ends(std::string&str,const std::vector<std::string>&end){
	bool removed=false;
	for(const auto&item:end)
		if(str_remove_end(str,item))
			removed=true;
	return removed;
}

void str_remove_all_end(std::string&str,const std::string&end){
	while(str_remove_end(str,end));
}

void str_remove_all_ends(std::string&str,const std::vector<std::string>&end){
	for(const auto&item:end)
		str_remove_all_end(str,item);
}

void str_trim_end(std::string&str){
	size_t len=str.length();
	size_t pos=len;
	while(pos>0&&std::isspace(str[pos-1]))pos--;
	if(pos<len)str.erase(pos);
}

void str_trim_start(std::string&str){
	size_t len=str.length();
	size_t pos=0;
	while(pos<len&&std::isspace(str[pos]))pos++;
	if(pos>0)str.erase(0,pos);
}

void str_trim(std::string&str){
	str_trim_end(str);
	str_trim_start(str);
}

std::string str_trim_end_to(const std::string&str){
	std::string s=str;
	str_trim_end(s);
	return s;
}

std::string str_trim_start_to(const std::string&str){
	std::string s=str;
	str_trim_start(s);
	return s;
}

std::string str_trim_to(const std::string&str){
	std::string s=str;
	str_trim(s);
	return s;
}

bool str_contains(const std::string&str1,const std::string&str2){
	if(str1.empty())return false;
	if(str2.empty())return true;
	return str1.find(str2)!=std::string::npos;
}

std::vector<std::string>str_split(const std::string&str,const std::string&sep){
	std::vector<std::string>ret{};
	std::string::size_type pos=0;
	while(true){
		auto idx=str.find(sep,pos);
		if(idx==std::string::npos){
			ret.push_back(str.substr(pos));
			break;
		}
		ret.push_back(str.substr(pos,idx-pos));
		pos=idx+sep.length();
	}
	return ret;
}

std::vector<std::string>str_split(const std::string&str,char sep){
	std::vector<std::string>ret{};
	std::string::size_type pos=0;
	while(true){
		auto idx=str.find(sep,pos);
		if(idx==std::string::npos){
			ret.push_back(str.substr(pos));
			break;
		}
		ret.push_back(str.substr(pos,idx-pos));
		pos=idx+1;
	}
	return ret;
}

std::map<std::string,std::string>parse_environ(const std::string&cont,char lsep,char csep,bool comment){
	std::map<std::string,std::string>ret{};
	for(auto line:str_split(cont,lsep)){
		str_trim(line);
		if(line.empty())continue;
		if(comment&&line[0]=='#')continue;
		auto idx=line.find(csep);
		if(idx==std::string::npos)continue;
		auto key=str_trim_to(line.substr(0,idx));
		auto val=str_trim_to(line.substr(idx+1));
		ret[key]=val;
	}
	return ret;
}

std::multimap<std::string,std::string>parse_multi_environ(const std::string&cont,char lsep,char csep,bool comment){
	std::multimap<std::string,std::string>ret{};
	for(auto line:str_split(cont,lsep)){
		str_trim(line);
		if(line.empty())continue;
		if(comment&&line[0]=='#')continue;
		auto idx=line.find(csep);
		if(idx==std::string::npos)continue;
		auto key=str_trim_to(line.substr(0,idx));
		auto val=str_trim_to(line.substr(idx+1));
		ret.insert({key,val});
	}
	return ret;
}

int hex2dec(char hex){
	if(hex>='0'&&hex<='9')return hex-'0';
	if(hex>='a'&&hex<='f')return hex-'a'+0xA;
	if(hex>='A'&&hex<='F')return hex-'A'+0xA;
	throw InvalidArgument("invalid hex value");
}

char dec2hex(int dec,bool upper){
	if(dec<0||dec>15)throw InvalidArgument("invalid dec value");
	if(dec<10)return '0'+dec;
	return (upper?'A':'a')+(dec-0xA);
}

std::string vssprintf(const char*fmt,va_list va){
	int ret;
	char buff[1024];
	if(!fmt)throw InvalidArgument("invalid fmt");
	ret=vsnprintf(buff,sizeof(buff),fmt,va);
	if(ret<0)throw ErrnoError("vsnprintf failed");
	if(ret>=(int)sizeof(buff)){
		char*buf=nullptr;
		ret=vasprintf(&buf,fmt,va);
		if(ret<0)throw ErrnoError("vasprintf failed");
		if(!buf)throw RuntimeError("vasprintf return null");
		return std::string(buf,buf+ret);
	}
	return std::string(buff,buff+ret);
}

std::string ssprintf(const char*fmt,...){
	va_list va;
	va_start(va,fmt);
	auto ret=vssprintf(fmt,va);
	va_end(va);
	return ret;
}

std::string str_get_env(const std::string&str){
	auto ret=getenv(str.c_str());
	return ret?ret:"";
}

std::string char_to_string(char c){
	std::string ret{};
	ret+=c;
	return ret;
}

std::vector<std::string>split(const std::string&str,const std::string&sep){
	std::vector<std::string>ret{};
	std::string::size_type pos=0;
	while(true){
		auto idx=str.find(sep,pos);
		if(idx==std::string::npos){
			ret.push_back(str.substr(pos));
			break;
		}
		ret.push_back(str.substr(pos,idx-pos));
		pos=idx+sep.length();
	}
	return ret;
}

std::vector<std::string>split(const std::string&str,char sep){
	std::vector<std::string>ret{};
	std::string::size_type pos=0;
	while(true){
		auto idx=str.find(sep,pos);
		if(idx==std::string::npos){
			ret.push_back(str.substr(pos));
			break;
		}
		ret.push_back(str.substr(pos,idx-pos));
		pos=idx+1;
	}
	return ret;
}

std::vector<std::string>string_array_to_vector(const char**arr,size_t count){
	std::vector<std::string>ret{};
	if(!arr)return ret;
	for(size_t i=0;arr[i]&&(count==0||i<count);i++)
		ret.emplace_back(arr[i]);
	return ret;
}

std::string vector_to_string(
	const std::vector<std::string>&strs,
	const std::string&sep
){
	std::string ret{};
	bool first=true;
	for(const auto&str:strs){
		if(!first)ret+=sep;
		ret+=str;
		first=false;
	}
	return ret;
}

std::unique_ptr<char*[],decltype(&free)>vector_to_string_array(
	const std::vector<std::string>&strs
){
	size_t idx_len=sizeof(char*);
	size_t str_len=0;
	for(const auto&str:strs){
		idx_len+=sizeof(char*);
		str_len+=str.length()+1;
	}
	auto len=idx_len+str_len;
	auto area=malloc(len);
	if(!area)throw RuntimeError("allocate failed");
	memset(area,0,len);
	auto x_idx=(char**)area;
	auto x_str=(char*)area+idx_len;
	size_t idx=0;
	for(const auto&str:strs){
		x_idx[idx++]=x_str;
		strcpy(x_str,str.c_str());
		x_str+=str.length()+1;
	}
	return {x_idx,&free};
}

std::map<std::string,std::string>string_array_to_map(
	const char**arr,
	const std::string&sep
){
	std::map<std::string,std::string>ret{};
	if(!arr)return ret;
	for(size_t i=0;arr[i];i++){
		std::string str=arr[i];
		if(str.empty())continue;
		auto idx=str.find(sep);
		if(idx==std::string::npos)continue;
		auto key=str.substr(0,idx);
		auto val=str.substr(idx+sep.length());
		if(key.empty())continue;
		ret[key]=val;
	}
	return ret;
}

std::unique_ptr<char*[],decltype(&free)>map_to_string_array(
	const std::map<std::string,std::string>&strs,
	const std::string&sep
){
	std::vector<std::string>list{};
	for(const auto&[key,value]:strs)
		list.push_back(key+sep+value);
	return vector_to_string_array(list);
}

std::string str_unicode_to_utf8(char32_t u){
	std::string utf8;
	if(u<=0x7F){
		utf8.push_back(static_cast<char>(u));
	}else if(u<=0x7FF){
		utf8.push_back(static_cast<char>(0xC0|((u>>6)&0x1F)));
		utf8.push_back(static_cast<char>(0x80|(u&0x3F)));
	}else if(u<=0xFFFF){
		utf8.push_back(static_cast<char>(0xE0|((u>>12)&0x0F)));
		utf8.push_back(static_cast<char>(0x80|((u>>6)&0x3F)));
		utf8.push_back(static_cast<char>(0x80|(u&0x3F)));
	}else if(u<=0x10FFFF){
		utf8.push_back(static_cast<char>(0xF0|((u>>18)&0x07)));
		utf8.push_back(static_cast<char>(0x80|((u>>12)&0x3F)));
		utf8.push_back(static_cast<char>(0x80|((u>>6)&0x3F)));
		utf8.push_back(static_cast<char>(0x80|(u&0x3F)));
	}else throw InvalidArgument("invalid unicode code point");
	return utf8;
}

bool check_ident_char(char c,bool first){
	if(c=='_')return true;
	if(std::isupper(c))return true;
	if(std::islower(c))return true;
	if(!first&&std::isdigit(c))return true;
	return false;
}

bool check_ident_string(const std::string&str){
	for(size_t i=0;i<str.length();i++)
		if(!check_ident_char(str[i],i==0))
			return false;
	return true;
}

void string_to_upper(std::string&str){
	std::transform(str.begin(),str.end(),str.begin(),::toupper);
}

void string_to_lower(std::string&str){
	std::transform(str.begin(),str.end(),str.begin(),::tolower);
}

bool string_is_true(std::string str){
	string_to_lower(str);
	if(str=="1")return true;
	if(str=="ok")return true;
	if(str=="on")return true;
	if(str=="yes")return true;
	if(str=="true")return true;
	if(str=="always")return true;
	if(str=="enable")return true;
	if(str=="enabled")return true;
	return false;
}

bool string_is_false(std::string str){
	string_to_lower(str);
	if(str=="0")return true;
	if(str=="no")return true;
	if(str=="off")return true;
	if(str=="false")return true;
	if(str=="never")return true;
	if(str=="disable")return true;
	if(str=="disabled")return true;
	return false;
}

bool is_number(const std::string&str,int base){
	if(str.empty())return false;
	char*end=nullptr;
	errno=0;
	strtoll(str.c_str(),&end,base);
	if(errno!=0)return false;
	if(!end||*end)return false;
	return true;
}
