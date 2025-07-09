#include<format>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<sys/sysmacros.h>
#include<linux/loop.h>
#include"loop.h"
#include"error.h"
#include"cleanup.h"
#include"fs-utils.h"

int loop_get_free(){
	int ctrl_fd=open("/dev/loop-control",O_RDWR|O_CLOEXEC);
	if(ctrl_fd<0)throw ErrnoError("failed to open /dev/loop-control");
	cleanup_func ctrl_close(std::bind(&close,ctrl_fd));
	auto id=ioctl(ctrl_fd,LOOP_CTL_GET_FREE);
	if(id<0){
		if(errno!=ENODEV)throw ErrnoError("failed to get free loop device");
		id=ioctl(ctrl_fd,LOOP_CTL_ADD);
		if(id<0)throw ErrnoError("failed to add loop device");
	}
	return id;
}

std::string loop_find_device(int loop_id){
	if(loop_id<0)loop_id=loop_get_free();
	auto path=std::format("/dev/loop{}",loop_id);
	if(fs_exists(path))return path;
	mknod(path.c_str(),S_IFBLK|0660,makedev(7,loop_id));
	if(fs_exists(path))return path;
	throw ErrnoError("loop device {} not found",path);
}

static int loop_open(int&loop_id){
	if(loop_id<0)loop_id=loop_get_free();
	auto path=loop_find_device(loop_id);
	auto fd=open(path.c_str(),O_RDWR|O_CLOEXEC);
	if(fd<0)throw ErrnoError("failed to open loop device {}",path);
	return fd;
}

int loop_set_fd(int file_fd,int loop_id){
	auto loop_fd=loop_open(loop_id);
	cleanup_func loop_close(std::bind(&close,loop_fd));
	auto ret=ioctl(loop_fd,LOOP_SET_FD,file_fd);
	if(ret<0)throw ErrnoError("failed to setup loop fd for loop{}",loop_id);
	return loop_id;
}

int loop_set_file(const std::string&file,int loop_id){
	auto file_fd=open(file.c_str(),O_RDWR|O_CLOEXEC);
	if(file_fd<0)file_fd=open(file.c_str(),O_RDONLY|O_CLOEXEC);
	if(file_fd<0)throw ErrnoError("failed to open {}",file);
	cleanup_func file_close(std::bind(&close,file_fd));
	return loop_set_fd(file_fd,loop_id);
}

void loop_detach(int loop_id){
	if(loop_id<0)throw ErrnoError("invalid loop device id");
	auto loop_fd=loop_open(loop_id);
	cleanup_func loop_close(std::bind(&close,loop_fd));
	auto ret=ioctl(loop_fd,LOOP_CLR_FD);
	if(ret<0)throw ErrnoError("failed to detach loop fd for loop{}",loop_id);
}
