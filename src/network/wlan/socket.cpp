#include"internal.h"
#include"configs.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"path-utils.h"
#include"error.h"
#include<poll.h>
#include<format>
#include<unistd.h>
#include<sys/un.h>

std::string get_wpa_supplicant_dir(){
	std::string wpa_dir="/run/wpa_supplicant";
	if(auto v=config["nanodistro"]["network"]["wlan"]["wpa-supplicant"]["ctrl"])
		wpa_dir=v.as<std::string>();
	return wpa_dir;
}

void wlan_client_impl::open(const std::string&dev){
	if(dev.empty())throw InvalidArgument("empty device name");
	size_t i=0;
	do{
		local_sock=std::format("/tmp/.nanodistro-wpa-{}-{}.sock",getpid(),i++);
	}while(fs_exists(local_sock));
	auto path=path_join(get_wpa_supplicant_dir(),dev);
	if(!fs_exists(path))start_wpa_supplicant(dev);
	fd=::socket(AF_UNIX,SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK,0);
	if(fd<0)throw ErrnoError("failed to create socket");
	sockaddr_un addr{},local{};
	addr.sun_family=AF_UNIX;
	local.sun_family=AF_UNIX;
	strncpy(local.sun_path,local_sock.c_str(),sizeof(local.sun_path)-1);
	strncpy(addr.sun_path,path.c_str(),sizeof(addr.sun_path)-1);
	if(::bind(fd,(sockaddr*)&local,sizeof(local))<0)
		throw ErrnoError("failed to bind socket");
	if(::connect(fd,(sockaddr*)&addr,sizeof(addr))<0)
		throw ErrnoError("failed to connect to socket");
	this->dev=dev;
	auto f=std::bind(&wlan_client_impl::on_event,this,std::placeholders::_1);
	poll_id=event_loop::push_handler(fd,EPOLLIN,f);
	run("ATTACH");
}

void wlan_client_impl::close(){
	if(fd<0)return;
	if(poll_id>0){
		event_loop::pop_handler(poll_id);
		poll_id=0;
	}
	try{send("DETACH");}catch(...){}
	::close(fd);
	fd=-1;
	if(!local_sock.empty())
		::unlink(local_sock.c_str());
}

std::shared_ptr<wlan_client>wlan_client::create(const std::string&dev){
	auto d=std::make_shared<wlan_client_impl>();
	d->open(dev);
	return d;
}

void wlan_client_impl::send_(const std::string&cmd){
	if(fd<0)throw RuntimeError("socket not opened");
	ssize_t ret;
	const char*d=cmd.data();
	size_t len=cmd.length();
	while(len>0){
		ret=::send(fd,d,len,0);
		if(ret<0){
			if(errno==EINTR)continue;
			if(errno==EAGAIN){
				pollfd pfd={.fd=fd,.events=POLLOUT,.revents=0};
				auto r=::poll(&pfd,1,-1);
				if(r>=0)continue;
			}
			throw ErrnoError("failed to send data");
		}
		if(ret==0)throw RuntimeError("send data reached EOF");
		d+=ret,len-=ret;
	}
}

std::string wlan_client_impl::exec_(const std::string&cmd){
	send_(cmd);
	while(true){
		auto ret=str_trim_to(recv_());
		return ret;
	}
}

std::string wlan_client_impl::recv(){
	std::lock_guard<std::mutex>g(lock);
	return recv_();
}

void wlan_client_impl::send(const std::string&cmd){
	std::lock_guard<std::mutex>g(lock);
	return send_(cmd);
}

std::string wlan_client_impl::exec(const std::string&cmd){
	std::lock_guard<std::mutex>g(lock);
	return exec_(cmd);
}

void wlan_client_impl::run(const std::string&cmd,const std::string&except){
	auto ret=exec(cmd);
	if(ret!=except)throw RuntimeError(
		"command {} result mismatch (expect {}, got {})",
		cmd,except,ret
	);
}
