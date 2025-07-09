#include"error.h"
#include<format>
#include<cstring>

RuntimeErrorImpl::RuntimeErrorImpl(
	const std::string&msg,
	const std::source_location&loc
):RuntimeErrorImpl(msg,log_location(loc)){
}

RuntimeErrorImpl::RuntimeErrorImpl(
	const std::string&msg,
	const log_location&loc
):msg(msg),loc(loc){
	vmsg=msg;
	if(loc.line>0)vmsg+=std::format(" at {}",loc.tostring());
}

const char*RuntimeErrorImpl::what()const noexcept{
	return vmsg.c_str();
}

InvalidArgumentImpl::InvalidArgumentImpl(
	const std::string&msg,const std::source_location&loc
):RuntimeErrorImpl(msg,loc){}

InvalidArgumentImpl::InvalidArgumentImpl(
	const std::string&msg,const log_location&loc
):RuntimeErrorImpl(msg,loc){}

ErrnoErrorImpl::ErrnoErrorImpl(
	int e,const std::string&msg,const std::source_location&loc
):ErrnoErrorImpl(e,msg,log_location(loc)){}

ErrnoErrorImpl::ErrnoErrorImpl(
	int e,const std::string&msg,const log_location&loc
):RuntimeErrorImpl(msg,loc),err(e){
	if(err<0)err=-err;
	if(err>0){
		auto es=std::format(": {}",strerror(err));
		this->vmsg+=es,this->msg+=es;
	}
}
