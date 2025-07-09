#include"cleanup.h"
#include"fs-utils.h"
#include"std-utils.h"
#include"str-utils.h"
#include"path-utils.h"
#include"error.h"
#include<climits>
#include<poll.h>
#include<fcntl.h>
#include<cassert>
#include<dirent.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<sys/resource.h>

std::string fs_readlink(const std::string&link){
	char buff[4096]{};
	auto ret=::readlink(link.c_str(),buff,sizeof(buff));
	if(ret<0)throw ErrnoError("readlink {} failed",link);
	return std::string(buff,buff+ret);
}

bool fs_exists(const std::string&path){
	return access(path.c_str(),F_OK)==0;
}

bool fs_is_type(const std::string&path,mode_t type){
	struct stat st{};
	auto ret=fstatat(AT_FDCWD,path.c_str(),&st,AT_SYMLINK_NOFOLLOW);
	if(ret!=0)return false;
	return ((st.st_mode&S_IFMT)&(type&S_IFMT))!=0;
}

bool fs_is_folder(const std::string&path){return fs_is_type(path,S_IFDIR);}
bool fs_is_char(const std::string&path){return fs_is_type(path,S_IFCHR);}
bool fs_is_block(const std::string&path){return fs_is_type(path,S_IFBLK);}
bool fs_is_file(const std::string&path){return fs_is_type(path,S_IFREG);}
bool fs_is_fifo(const std::string&path){return fs_is_type(path,S_IFIFO);}
bool fs_is_link(const std::string&path){return fs_is_type(path,S_IFLNK);}
bool fs_is_socket(const std::string&path){return fs_is_type(path,S_IFSOCK);}
bool fs_is_device(const std::string&path){return fs_is_type(path,S_IFCHR|S_IFBLK);}

std::string fs_resolvelink(const std::string&link){
	int cnt=0;
	std::string real_path=link;
	while(fs_is_link(real_path)){
		auto next_path=fs_readlink(real_path);
		if(cnt++>16)throw RuntimeError("bad link {}",real_path);
		real_path=next_path;
	}
	return real_path;
}

int fs_get_max_fd(){
	rlim_t m;
	rlimit rl;
	if(getrlimit(RLIMIT_NOFILE,&rl)<0)return INT_MAX;
	m=std::max(rl.rlim_cur,rl.rlim_max);
	if(m<FD_SETSIZE)return FD_SETSIZE-1;
	if(m==RLIM_INFINITY||m>INT_MAX)return INT_MAX;
	return (int)(m-1);
}

void fs_close_all_fds(const std::vector<int>&exclude){
	try{
		auto dir=opendir("/proc/self/fd");
		if(!dir)return;
		auto dfd=dirfd(dir);
		while(auto e=readdir(dir))try{
			if(e->d_name[0]=='.')continue;
			if(e->d_type!=DT_LNK)continue;
			int fd=std::stoi(e->d_name);
			if(std_contains(exclude,fd)&&fd!=dfd)continue;
			close(fd);
		}catch(...){}
		closedir(dir);
	}catch(...){
		for(int fd=0;fd<fs_get_max_fd();fd++)
			if(!std_contains(exclude,fd))
				close(fd);
	}
}

std::string fs_read_all(const std::string&path,size_t page){
	std::string data{};
	struct stat st{};
	int fd=open(path.c_str(),O_RDONLY);
	if(fd<0)throw ErrnoError("failed to open {}",path);
	cleanup_func autoclose(std::bind(&close,fd));
	if(fstat(fd,&st)==0&&st.st_size>0)
		data.resize(st.st_size);
	size_t real=0;
	ssize_t ret;
	while(true){
		assert(real<=data.size());
		if(data.size()-real<page)
			data.resize(std::max(data.size(),real)+page);
		ret=read(fd,data.data()+real,page);
		if(ret==0)break;
		if(ret<0){
			if(errno==EINTR)continue;
			throw ErrnoError("failed to read {} at {}",path,real);
		}
		real+=ret;
	}
	data.resize(real);
	return data;
}

void fs_write_all(const std::string&path,const std::string&data,int flags,mode_t mode){
	int fd=open(path.c_str(),flags,mode);
	if(fd<0)throw ErrnoError("failed to open {}",path);
	cleanup_func autoclose(std::bind(&close,fd));
	size_t written=0;
	while(written<data.size()){
		auto to_write=data.size()-written;
		auto ret=write(fd,data.data()+written,to_write);
		if(ret<0){
			if(errno==EINTR)continue;
			throw ErrnoError("failed to write {} at {}",path,written);
		}
		if(ret==0)break;
		written+=ret;
	}
	if(written<data.size())
		throw RuntimeError("failed to write {} at {}",path,written);
}

void fs_write_all(const std::string&path,const std::string&data){
	fs_write_all(path,data,O_WRONLY|O_TRUNC|O_CREAT,0644);
}

void fs_write_numlf(const std::string&path,size_t num,int flags,mode_t mode){
	fs_write_all(path,std::format("{}\n",num),flags,mode);
}

void fs_write_numlf(const std::string&path,size_t num){
	fs_write_all(path,std::format("{}\n",num));
}

void fs_append_all(const std::string&path,const std::string&data){
	fs_write_all(path,data,O_WRONLY|O_APPEND|O_CREAT,0644);
}

void fs_append_numlf(const std::string&path,size_t num){
	fs_append_all(path,std::format("{}\n",num));
}

std::string fs_simple_read(const std::string&dir,const std::string&file){
	return fs_simple_read(path_join(dir,file));
}

std::string fs_simple_read(const std::string&file){
	try{
		auto ret=fs_read_all(file);
		str_trim(ret);
		return ret;
	}catch(std::exception&exc){
		log_exception(exc,"error reading {}",file);
		return {};
	}
}

int xioctl_(int fd,const std::string&name,long request,long arg){
	if(fd<0)throw RuntimeError("invalid fd {}",fd);
	int ret=ioctl(fd,request,arg);
	if(ret<0)throw ErrnoError("ioctl {} failed",name);
	return ret;
}

size_t full_read(int fd,void*buf,size_t size){
	size_t off=0;
	while(size-off>0){
		auto ret=read(fd,(char*)buf+off,size-off);
		if(ret<0){
			if(errno==EINTR)continue;
			if(errno==EAGAIN)break;
			throw ErrnoError("read failed");
		}
		if(ret==0)break;
		off+=ret;
	}
	return off;
}

void io_wait(int fd,int events,int timeout){
	while(true){
		pollfd pfd{};
		pfd.fd=fd;
		pfd.events=events;
		auto ret=poll(&pfd,1,timeout);
		if(ret<0){
			if(errno==EINTR)continue;
			throw ErrnoError("poll failed");
		}
		if(ret==0)throw RuntimeError("poll timeout");
	}
}

void full_write(int fd,const void*buf,size_t size){
	size_t off=0;
	while(size-off>0){
		auto ret=write(fd,(char*)buf+off,size-off);
		if(ret<0){
			if(errno==EINTR)continue;
			if(errno==EAGAIN){
				io_wait(fd,POLLOUT,100);
				continue;
			}
			throw ErrnoError("write failed");
		}
		if(ret==0&&off<size)
			throw RuntimeError("write {} at {} reached EOF",size,off);
		off+=ret;
	}
	if(off<size)throw RuntimeError("write {} at {} failed",size,off);
}
