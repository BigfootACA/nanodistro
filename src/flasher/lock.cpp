#include<fcntl.h>
#include<sys/stat.h>
#include<sys/sysmacros.h>
#include"internal.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"error.h"

static pid_t get_lock_owner(int fd){
	struct stat st;
	try{
		if(fstat(fd,&st)<0)return -1;
		auto match=std::format(
			"{:02x}:{:02x}:{}",
			major(st.st_dev),
			minor(st.st_dev),
			st.st_ino
		);
		auto f=[](auto str){return str.empty();};
		auto cont=fs_read_all("/proc/locks");
		for(auto line:str_split(cont,'\n')){
			auto fields=str_split(line,' ');
			auto r=std::remove_if(fields.begin(),fields.end(),f);
			fields.erase(r,fields.end());
			if(fields.size()<6)continue;
			if(fields[1]!="POSIX")continue;
			if(fields[5]!=match)continue;
			size_t idx=0;
			auto ret=std::stoi(fields[4],&idx);
			if(idx!=fields[4].length())continue;
			return ret;
		}
	}catch(...){}
	return -1;
}

void flasher_lock(){
	if(flasher.lock_fd<0){
		int fd=open(LOCK_PATH,O_RDWR|O_CREAT|O_CLOEXEC,0666);
		if(fd<0)throw RuntimeError("failed to open lock file");
		flasher.lock_fd=fd;
	}
	flock fl{};
	fl.l_type=F_WRLCK;
	fl.l_whence=SEEK_SET;
	if(fcntl(flasher.lock_fd,F_SETLK,&fl)>=0)return;
	int e=errno;
	auto pid=get_lock_owner(flasher.lock_fd);
	if(pid>0)log_warning("flasher is already running by pid {}",pid);
	throw ErrnoErrorWith(e,"failed to lock flasher");	
}

void flasher_unlock(){
	if(flasher.lock_fd<0)return;
	flock fl{};
	fl.l_type=F_UNLCK;
	fl.l_whence=SEEK_SET;
	if(fcntl(flasher.lock_fd,F_SETLK,&fl)<0)
		log_warning("failed to unlock flasher: {}",strerror(errno));
	close(flasher.lock_fd);
	flasher.lock_fd=-1;
}
