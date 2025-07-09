#ifndef INTERNAL_H
#define INTERNAL_H
#include<string>
#include<csignal>
#include<memory>
#include<libmount/libmount.h>
#include"cgroup.h"
#include"configs.h"
#include"process.h"
extern int flasher_do_main();
extern void flasher_cleanup();
extern void flasher_lock();
extern void flasher_unlock();
extern void start_netblk();
#define LOCK_PATH "/tmp/.nanodistro-flasher.lock"
struct flasher_context{
	int lock_fd=-1;
	std::string folder{};
	std::string img_device{};
	std::shared_ptr<cgroup_pids>pids=nullptr;
	YAML::Node img_manifest{};
	Json::Value netblk_result{};
	std::shared_ptr<process>script=nullptr;
	std::shared_ptr<process>netblk=nullptr;
};
extern flasher_context flasher;
#endif
