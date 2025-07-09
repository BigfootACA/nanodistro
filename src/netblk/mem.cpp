#include<cstring>
#include<sys/stat.h>
#include"internal.h"
#include"error.h"
#include"fs-utils.h"

class netblk_backend_mem:public netblk_backend{
	public:
		netblk_backend_mem()=default;
		inline std::string get_name()const override{return "mem";}
		inline size_t get_size()const override{return data.size();}
		inline mode_t access()const override{return S_IROTH|S_IWOTH;}
		size_t read(size_t offset,void*buf,size_t size)override;
		size_t write(size_t offset,const void*buf,size_t size)override;
		std::string data;
};

std::shared_ptr<netblk_backend>netblk_backend_create_mem(const Json::Value&opts){
	auto backend=std::make_shared<netblk_backend_mem>();
	auto file=opts["file"].asString();
	if(file.empty())throw RuntimeError("mem backend file not specified");
	if(!fs_exists(file))throw RuntimeError("mem backend file {} not exists",file);
	backend->data=fs_read_all(file);
	return backend;
}

size_t netblk_backend_mem::read(size_t offset,void*buf,size_t size){
	if(size==0)return 0;
	if(!buf)throw ErrnoErrorWith(EINVAL,"invalid buffer");
	if(offset>=get_size())
		throw ErrnoErrorWith(ERANGE,"read out of range");
	auto toread=std::min(get_size()-offset,size);
	memcpy(buf,(uint8_t*)data.data()+offset,toread);
	return toread;
}

size_t netblk_backend_mem::write(size_t offset,const void*buf,size_t size){
	if(size==0)return 0;
	if(!buf)throw ErrnoErrorWith(EINVAL,"invalid buffer");
	if(offset>=get_size())
		throw ErrnoErrorWith(ERANGE,"write out of range");
	auto towrite=std::min(get_size()-offset,size);
	memcpy((uint8_t*)data.data()+offset,buf,towrite);
	return towrite;
}
