#include"url.h"
#include"error.h"

void url::clear(){
	set_scheme("");
	set_username_decoded("");
	set_password_decoded("");
	set_host_decoded("");
	set_port(-1);
	set_path_decoded("");
	set_query("");
	set_fragment("");
}

void url::from(const url*url){
	if(!url)return;
	this->set_scheme(url->get_scheme());
	this->set_username_decoded(url->get_username());
	this->set_password_decoded(url->get_password());
	this->set_host_decoded(url->get_host());
	this->set_port(url->get_port());
	this->set_path_decoded(url->get_path());
	this->set_query(url->get_query());
	this->set_fragment(url->get_fragment());
}

int url::compare(const std::string&u)const{
	url*nu;
	try{nu=new url(u);}catch(...){return -1;}
	return this->compare(nu);
}

int url::compare(const url*that)const{
	try{
		auto l=this->to_string();
		auto r=that->to_string();
		return l.compare(r);
	}catch(...){
		return -1;
	}
}

void url::set_port(const std::string&val){
	size_t idx=0;
	auto r=std::stoi(val,&idx);
	if(idx!=val.length())throw RuntimeError("bad port");
	set_port(r);
}

bool url::go_back(){
	if(is_in_top())return false;
	size_t len=path.length();
	if(path.ends_with('/'))path.pop_back();
	while(!path.ends_with('/')&&!path.empty())
		path.pop_back();
	return path.length()!=len;
}

url url::relative(const std::string&path)const{
	url nu(*this);
	if(path.find("://")!=std::string::npos)
		return url(path);
	if(path.starts_with('?')){
		if(!nu.query.empty())
			nu.query+='&';
		nu.query+=path.substr(1);
		return nu;
	}
	if(path.starts_with('#')){
		nu.fragment=path.substr(1);
		return nu;
	}
	nu.query.clear();
	nu.fragment.clear();
	std::string new_path;
	if(path.starts_with('/'))new_path=path;
	else if(!nu.path.empty()){
		new_path=nu.path;
		if(!new_path.ends_with("/")){
			auto pos=new_path.rfind('/');
			if(pos!=std::string::npos)
				new_path.erase(pos+1,std::string::npos);
		}
		new_path+=path;
	}else new_path="/"+path;
	std::vector<std::string>parts,stack;
	size_t start=0,end;
	while((end=new_path.find('/',start))!=std::string::npos){
		auto part=new_path.substr(start,end-start);
		if(!part.empty())parts.push_back(part);
		start=end+1;
	}
	if(start<new_path.size())
		parts.push_back(new_path.substr(start));
	for(const auto&part:parts){
		if(part==".."){
			if(!stack.empty())stack.pop_back();
		}else if(part!="."&&!part.empty()){
			stack.push_back(part);
		}
	}
	std::string resolved_path="/";
	for(size_t i=0;i<stack.size();i++){
		if(i)resolved_path+='/';
		resolved_path+=stack[i];
	}
	if(new_path.ends_with("/")&&!resolved_path.ends_with("/"))
		resolved_path+="/";
	nu.set_path(resolved_path);
	return nu;
}
