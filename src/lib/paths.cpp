#include"path-utils.h"
#include"str-utils.h"
#include"fs-utils.h"
#include"error.h"

std::string path_basename(const std::string&str){
	std::string path=str;
	str_remove_end(path,"/");
	auto ret=path.rfind('/');
	if(ret!=std::string::npos)
		path=path.substr(ret+1);
	return path;
}

std::string path_dirname(const std::string&str){
	std::string path=str;
	str_remove_end(path,"/");
	auto ret=path.rfind('/');
	if(ret!=std::string::npos)
		path.erase(ret);
	return path;
}

std::string path_join(const std::string&dir,const std::string&file){
	std::string path{};
	if(file.starts_with('/'))return file;
	path+=dir;
	if(!path.ends_with('/'))path+='/';
	path+=file;
	return path;
}

std::string path_find_exec(const std::string&exe){
	if(exe.find('/')==std::string::npos){
		for(auto&dir:str_split(str_get_env("PATH"),':')){
			auto path=path_join(dir,exe);
			if(fs_exists(path))return path;
		}
		throw RuntimeError("command {} not found",exe);
	}else return exe;
}

std::string path_get_self(){
	return fs_readlink("/proc/self/exe");
}
