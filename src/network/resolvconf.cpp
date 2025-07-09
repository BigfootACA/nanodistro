#include"net-utils.h"
#include"str-utils.h"
#include"error.h"
#include<format>

void resolv_conf::parse_line(const std::string&line){
	auto data=str_trim_to(line);
	if(data.empty()||data.starts_with('#'))return;
	auto params=str_split(data,' ');
	if(params[0]=="search")search=params[1];
	else if(params[0]=="nameserver"){
		if(params.size()<2)return;
		if(params[1].empty())return;
		nameservers.push_back(params[1]);
	}else if(params[0]=="options"){
		for(size_t i=1;i<params.size();i++)
			options.push_back(params[i]);
	}
}

std::string resolv_conf::to_file()const{
	std::string ret{};
	if(!search.empty())
		ret+=std::format("search {}\n",search);
	for(auto&ns:nameservers)
		ret+=std::format("nameserver {}\n",ns);
	if(!options.empty()){
		ret+=std::format("options ");
		for(size_t i=0;i<options.size();i++){
			if(i>0)ret+=' ';
			ret+=options[i];
		}
		ret+='\n';
	}
	return ret;
}

void resolv_conf::parse_file(const std::string&content){
	for(auto&line:str_split(content,'\n'))
		parse_line(line);
}

void resolv_conf::load_file(const std::string&path){
	FILE*fp=fopen(path.c_str(),"r");
	if(!fp)throw ErrnoError("failed to open file {}",path);
	char buf[4096];
	while(fgets(buf,sizeof(buf),fp))
		parse_line(buf);
	fclose(fp);
}

void resolv_conf::save_file(const std::string&path)const{
	FILE*fp=fopen(path.c_str(),"w");
	if(!fp)throw ErrnoError("failed to open file {}",path);
	auto data=to_file();
	if(fwrite(data.data(),1,data.length(),fp)<0)
		throw ErrnoError("failed to write file {}",path);
	fclose(fp);
}

void resolv_conf::remove(const std::string&path)const{
	unlink(path.c_str());
}
