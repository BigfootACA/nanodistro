#include<fcntl.h>
#include<sys/ioctl.h>
#include"internal.h"
#include"std-utils.h"
#include"fs-utils.h"
#include"error.h"

void evdev_device::init_device(){
	if(fd>=0)return;
	if(path.empty())throw RuntimeError("device path is empty");
	fd=open(path.c_str(),O_RDONLY|O_NONBLOCK|O_CLOEXEC);
	if(fd<0)throw ErrnoError("failed to open {}",path);
	char name[4096];
	memset(name,0,sizeof(name));
	xioctl(fd,EVIOCGNAME(sizeof(name)),name);
	this->name=std::string(name,strnlen(name,sizeof(name)));
	#define get_info(_ev,_var)\
		if(ioctl(fd,(int)EVIOCGBIT(_ev,sizeof(bits.d._var)),bits.d._var)<0)\
			log_warning("failed to get {} bits for {}",#_ev,path);
	get_info(EV_SYN,syn)
	get_info(EV_KEY,key)
	get_info(EV_REL,rel)
	get_info(EV_ABS,abs)
	get_info(EV_MSC,msc)
	get_info(EV_SW,sw)
	get_info(EV_LED,led)
	get_info(EV_SND,snd)
	get_info(EV_FF,ff)
	if((bits.v.syn>>EV_ABS)&1)for(int i=0;i<ABS_MAX;i++)
		if(((bits.v.abs>>i)&1)&&ioctl(fd,EVIOCGABS(i),&abs[i])<0)
			log_warning("failed to get abs info for {}",i); 
	ioctl(fd,(int)EVIOCGPROP(sizeof(props)),&props);
	auto f1=std::bind(&evdev_device::on_event,this,std::placeholders::_1);
	poll_id=event_loop::push_handler(fd,EPOLLIN,f1);
	log_info("found device {} name {}",path,name);
}

void evdev_device::stop(){
	if(poll_id>0){
		event_loop::pop_handler(poll_id);
		poll_id=0;
	}
	auto idx=std_find_all(ctx->devices,this);
	if(idx!=ctx->devices.end())ctx->devices.erase(idx);
}

void evdev_device::on_event(event_handler_context*ev){
	if(ev->type!=type_events)return;
	if(ev->event&EPOLLIN)try{
		while(read_event());
	}catch(std::exception&exc){
		log_exception(exc,"process event failed");
		ev->ev->want_remove=true;
		poll_id=0;
		stop();
	}
}

bool evdev_device::read_event(){
	input_event ev{};
	ssize_t ret=read(fd,&ev,sizeof(ev));
	if(ret<0){
		if(errno==EAGAIN)return false;
		if(errno==EINTR)return true;
		throw ErrnoError("failed to read event from {}",path);
	}
	if(ret!=sizeof(ev)){
		log_warning("read event from {}: invalid size {}",path,ret);
		return false;
	}
	process_event(ev);
	ctx->trigger_read();
	return true;
}

evdev_device::~evdev_device(){
	stop();
}
