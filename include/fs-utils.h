#ifndef FS_UTILS_H
#define FS_UTILS_H
#include<string>
#include<vector>
#include<functional>
#include<sys/stat.h>
extern std::string fs_readlink(const std::string&link);
extern bool fs_exists(const std::string&path);
extern bool fs_is_type(const std::string&path,mode_t type);
extern bool fs_is_folder(const std::string&path);
extern bool fs_is_char(const std::string&path);
extern bool fs_is_block(const std::string&path);
extern bool fs_is_file(const std::string&path);
extern bool fs_is_fifo(const std::string&path);
extern bool fs_is_link(const std::string&path);
extern bool fs_is_socket(const std::string&path);
extern bool fs_is_device(const std::string&path);
extern std::string fs_resolvelink(const std::string&link);
extern int fs_get_max_fd();
extern void fs_close_all_fds(const std::vector<int>&exclude={0,1,2});
extern std::string fs_read_all(const std::string&path,size_t page=0x1000);
extern void fs_write_all(const std::string&path,const std::string&data,int flags,mode_t mode);
extern void fs_write_all(const std::string&path,const std::string&data);
extern void fs_write_numlf(const std::string&path,size_t num,int flags,mode_t mode);
extern void fs_write_numlf(const std::string&path,size_t num);
extern void fs_append_all(const std::string&path,const std::string&data);
extern void fs_append_numlf(const std::string&path,size_t num);
extern std::string fs_simple_read(const std::string&dir,const std::string&file);
extern std::string fs_simple_read(const std::string&file);
extern int xioctl_(int fd,const std::string&name,long request,long arg);
extern size_t full_read(int fd,void*buf,size_t size);
extern void io_wait(int fd,int events,int timeout);
extern void full_write(int fd,const void*buf,size_t size);
extern void fs_list_dir(const std::string&dir,std::function<bool(const std::string&name,int dt)>cb);
extern std::vector<std::string>fs_list_dir_all(const std::string&dir);
template<typename T>T xioctl_get_(int fd,const std::string&name,long request){
	T arg{};
	xioctl_(fd,name,request,(long)&arg);
	return arg;
}
#define xioctl(fd,req,arg) xioctl_(fd,#req,req,(long)arg)
#define xioctl_get(T,fd,req) xioctl_get_<T>(fd,#req,req)
#endif
