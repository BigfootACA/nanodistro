#include"internal.h"
#include"process.h"
#include"configs.h"
#include"fs-utils.h"
#include"path-utils.h"
#include"error.h"

static std::string get_wpa_supplicant_exec(){
	std::string wpa_exec="wpa_supplicant";
	if(auto v=config["nanodistro"]["network"]["wlan"]["wpa-supplicant"]["exec"])
		wpa_exec=v.as<std::string>();
	if(wpa_exec.starts_with('/'))wpa_exec=path_find_exec(wpa_exec);
	return wpa_exec;
}

void start_wpa_supplicant(const std::string&dev){
	if(dev.empty())throw InvalidArgument("empty device name");
	process p{};
	auto exec=get_wpa_supplicant_exec();
	auto dir=get_wpa_supplicant_dir();
	p.set_execute(exec);
	p.set_command({exec,"-i"+dev,"-C"+dir,"-Dnl80211",});
	log_info("starting wpa_supplicant at {} for {} ",exec,dev);
	p.start();
	auto last=time(nullptr);
	auto sock=path_join(dir,dev);
	while(!fs_exists(sock)){
		if(time(nullptr)-last>5)
			throw RuntimeError("wpa_supplicant start timeout");
		usleep(100000);
	}
	log_info("wpa_supplicant started for {} as {}",dev,p.pid);
}
