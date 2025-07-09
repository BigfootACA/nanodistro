#include"url.h"
#include<format>

size_t url::append_all(std::string&val)const{
	size_t len=val.length();
	append_scheme(val);
	append_hierarchical(val);
	append_query(val);
	append_fragment(val);
	return val.length()-len;
}

size_t url::append_origin(std::string&val)const{
	size_t len=val.length();
	append_host(val);
	append_port(val);
	return val.length()-len;
}

size_t url::append_hierarchical(std::string&val)const{
	size_t len=val.length();
	append_authority(val);
	append_path(val);
	return val.length()-len;
}

size_t url::append_user_info(std::string&val)const{
	size_t len=val.length();
	append_username(val);
	append_password(val);
	return val.length()-len;
}

size_t url::append_authority(std::string&val)const{
	size_t len=val.length();
	auto s=append_user_info(val);
	if(s>0)val+="@";
	append_origin(val);
	return val.length()-len;
}

size_t url::append_full_path(std::string&val)const{
	size_t len=val.length();
	append_path(val);
	append_query(val);
	append_fragment(val);
	return val.length()-len;
}

size_t url::append_scheme(std::string&val)const{
	size_t len=val.length();
	if(!scheme.empty())val+=scheme+":";
	val+="//";
	return val.length()-len;
}

size_t url::append_username(std::string&val)const{
	size_t len=val.length();
	if(!username.empty())val+=encode(username);
	return val.length()-len;
}

size_t url::append_password(std::string&val)const{
	size_t len=val.length();
	if(!password.empty())val+=":"+encode(password);
	return val.length()-len;
}

size_t url::append_host(std::string&val)const{
	size_t len=val.length();
	if(strpbrk(host.c_str(),"[]#?%"))
		val+=encode(host);
	else if(strpbrk(host.c_str(),":/@"))
		val+=std::format("[{0}]",host);
	else val+=host;
	return val.length()-len;
}

size_t url::append_port(std::string&val)const{
	size_t len=val.length();
	if(port>=0)val+=std::format(":{0:d}",port);
	return val.length()-len;
}

size_t url::append_path(std::string&val)const{
	size_t len=val.length();
	if(!path.starts_with('/'))val+="/";
	val+=url::encode(path,nullptr,"/");
	return val.length()-len;
}

size_t url::append_query(std::string&val)const{
	size_t len=val.length();
	if(!query.empty()){
		if(!val.empty())val+="?";
		val+=query;
	}
	return val.length()-len;
}

size_t url::append_fragment(std::string&val)const{
	size_t len=val.length();
	if(!fragment.empty()){
		if(!val.empty())val+="#";
		val+=fragment;
	}
	return val.length()-len;
}
