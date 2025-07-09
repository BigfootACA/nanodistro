#include"internal.h"
#include"error.h"
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<linux/fs.h>

std::shared_ptr<netblk_backend>netblk_backend::create(const std::string&type,const Json::Value&opts){
	std::string xtype=type;
	if(xtype.empty())xtype=opts["driver"].asString();
	if(xtype.empty())throw RuntimeError("no backend specified");
	for(auto&[name,creator]:netblk_backend_list)
		if(name==xtype&&creator)
			return creator(opts);
	throw RuntimeError("backend {} not supported",xtype);
}

bool netblk_implement::check_supported(){
	try{
		if(is_supported())return true;
		try_enable();
		if(is_supported())return true;
		log_info("{} is unsupported and cannot be enabled",get_name());
	}catch(std::exception&exc){
		log_exception(exc,"{} failed to enable",get_name());
	}
	return false;
}

std::shared_ptr<netblk_implement>netblk_implement::choose(const Json::Value&opts){
	std::shared_ptr<netblk_implement>ret=nullptr;
	auto try_driver=[&](bool prefer,auto name,auto creator){
		log_info("try netblk implement {}",name);
		if(!creator)return false;
		auto impl=creator(opts);
		if(prefer&&!impl->is_prefer())return false;
		if(!impl->check_supported())return false;
		impl->init();
		ret=impl;
		return true;
	};
	if(opts.isMember("driver")){
		auto driver=opts["driver"].asString();
		for(const auto&[name,creator]:netblk_implement_list)
			if(name==driver&&try_driver(false,name,creator))return ret;
		throw RuntimeError("netblk implement {} not supported",driver);
	}
	for(const auto&[name,creator]:netblk_implement_list)
		if(try_driver(true,name,creator))return ret;
	log_info("no prefer netblk implement found");
	for(const auto&[name,creator]:netblk_implement_list)
		if(try_driver(false,name,creator))return ret;
	throw RuntimeError("no supported netblk implement found");
}

bool netblk_backend::can_access(mode_t mode){
	return (access()&mode)==mode;
}

ssize_t netblk_backend::do_read(size_t offset,void*buf,size_t size){
	int ret=-EIO;
	if(!can_access(S_IROTH))return -EPERM;
	for(int i=0;i<std::max(1,retry);i++)try{
		auto ret=read(offset,buf,size);
		return (size_t)ret;
	}catch(ErrnoErrorImpl&exc){
		log_exception(exc,"{} read at {} length {} error",get_name(),offset,size);
		if(exc.err!=0)ret=-exc.err;
	}catch(std::exception&exc){
		log_exception(exc,"{} read at {} length {} error",get_name(),offset,size);
	}
	return ret;
}

ssize_t netblk_backend::do_write(size_t offset,const void*buf,size_t size){
	int ret=-EIO;
	if(!can_access(S_IWOTH))return -EPERM;
	for(int i=0;i<std::max(1,retry);i++)try{
		auto ret=write(offset,buf,size);
		return (size_t)ret;
	}catch(ErrnoErrorImpl&exc){
		log_exception(exc,"{} write at {} length {} error",get_name(),offset,size);
		if(exc.err!=0)ret=-exc.err;
	}catch(std::exception&exc){
		log_exception(exc,"{} write at {} length {} error",get_name(),offset,size);
	}
	return ret;
}

void netblk_implement::run(){
	sleep(1);
}

void netblk_implement::loop(){
	while(running)run();
}
