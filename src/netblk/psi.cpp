#include<fstream>
#include<sys/prctl.h>
#include"internal.h"

std::shared_ptr<psi_monitor>psi_monitor::instance=nullptr;
static bool stopped=false;

psi_monitor::psi_monitor(){
	thread=std::thread(std::bind(&psi_monitor::loop,this));
}

psi_monitor::~psi_monitor(){
	running=false;
	stopped=true;
	if(thread.joinable())thread.join();
}

void psi_monitor::on_low(){
	std::lock_guard<std::mutex>lock(mutex);
	for(auto cache:caches)
		cache->reduce_half();
}

void psi_monitor::check(){
	if(want_gc){
		want_gc=false;
		on_low();
		return;
	}
	try{
		std::string line;
		std::ifstream psi("/proc/pressure/memory");
		float avg10=0.0f;
		while(std::getline(psi,line)){
			std::smatch m;
			if(std::regex_search(line,m,re_avg10)){
				avg10=std::stof(m[1]);
				break;
			}
		}
		if(avg10>0.2f){
			on_low();
			return;
		}
	}catch(...){}
	try{
		std::string line,key{},unit{};
		std::ifstream meminfo("/proc/meminfo");
		size_t mem_total=0,mem_avail=0;
		while(std::getline(meminfo,line)){
			std::istringstream iss(line);
			size_t size=0;
			iss>>key>>size>>unit;
			size*=1024;
			if(line.find("MemTotal:")==0)mem_total=size;
			if(line.find("MemAvailable:")==0)mem_avail=size;
			if(mem_total>0&&mem_avail>0) break;
		}
		if(mem_total&&mem_avail&&mem_avail<mem_total/10){
			on_low();
			return;
		}
	}catch(...){}
}

void psi_monitor::loop(){
	prctl(PR_SET_NAME,"psi-monitor");
	while(running){
		sleep(1);
		check();
	}
}

void psi_monitor::add(block_cache*cache){
	std::lock_guard<std::mutex>lock(mutex);
	caches.push_back(cache);
}

void psi_monitor::remove(block_cache*cache){
	std::lock_guard<std::mutex>lock(mutex);
	caches.remove(cache);
}

std::shared_ptr<psi_monitor>psi_monitor::get(){
	if(!instance)instance=std::make_shared<psi_monitor>();
	return instance;
}

void psi_monitor::push(block_cache*cache){
	if(stopped)return;
	get()->add(cache);
}

void psi_monitor::pop(block_cache*cache){
	if(stopped)return;
	get()->remove(cache);
}
