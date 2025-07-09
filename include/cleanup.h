#ifndef LIB_CLEANUP_H
#define LIB_CLEANUP_H
#include<stdlib.h>
#include<unistd.h>
#include<functional>
template<typename t>
class cleanup{
	public:
		inline cleanup(t data,std::function<void(t)>callback)
			:data(data),callback(callback){}
		inline ~cleanup(){end();}
		inline t take(){called=true;return data;}
		inline void set(t val){data=val;}
		inline t get()const{return data;}
		inline bool is_called()const{return called;}
		inline void call(){callback(data);}
		inline void end(){if(!is_called())call();}
	private:
		t data;
		std::function<void(t)>callback;
		bool called=false;
};
class cleanup_func{
	public:
		inline cleanup_func(std::function<void(void)>callback)
			:callback(callback){}
		inline ~cleanup_func(){end();}
		inline void kill(){called=true;}
		inline bool is_called()const{return called;}
		inline void call(){callback();}
		inline void end(){if(!is_called())call();}
	private:
		std::function<void(void)>callback;
		bool called=false;
};
class fd_cleanup:public cleanup<int>{
	public:inline fd_cleanup(int data):cleanup<int>(data,[](int fd){
		if(fd>=0)close(fd);
	}){}
};
class pointer_cleanup:public cleanup<void*>{
	public:inline pointer_cleanup(void*data):cleanup<void*>(data,[](void*ptr){
		if(ptr)free(ptr);
	}){}
};
template<typename t>
class object_cleanup:public cleanup<t>{
	public:inline object_cleanup(t data):cleanup<t>(data,[](t d){
		if(d)delete d;
	}){}
};
#endif
