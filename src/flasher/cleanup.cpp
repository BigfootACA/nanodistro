#include<csignal>
#include<sys/wait.h>
#include"libmounts.h"
#include"internal.h"
#include"cleanup.h"
#include"path-utils.h"
#include"log.h"
#include"error.h"

static void umount_img(){
	int ret=-1;
	if(flasher.folder.empty())return;
	auto dir=path_join(flasher.folder,"mnt");
	log_info("umount image {}",dir);
	auto cdir=dir.c_str();
	auto ctx=mnt_new_context();
	if(!ctx)throw RuntimeError("failed to create mount context");
	cleanup_func umount_ctx(std::bind(mnt_free_context,ctx));
	mnt_context_set_target(ctx,cdir);
	if((ret=mnt_context_umount(ctx))>=0)return;
	log_warning(
		"failed to umount {}: {}",
		dir,libmount_strerror(ret)
	);
	mnt_context_enable_force(ctx,true);
	if((ret=mnt_context_umount(ctx))>=0)return;
	log_warning(
		"failed to force umount {}: {}",
		dir,libmount_strerror(ret)
	);
	umount2(cdir,0);
	umount2(cdir,MNT_FORCE|MNT_EXPIRE);
	umount2(cdir,MNT_FORCE|MNT_DETACH|MNT_EXPIRE);
}

void flasher_cleanup(){
	if(flasher.script)try{
		log_info("stopping script pid {}",flasher.script->pid);
		flasher.script->kill_wait();
	}catch(std::exception&exc){
		log_exception(exc,"failed to stop script");
	}
	flasher.script=nullptr;
	if(flasher.pids)try{
		log_info("stopping remaining tasks");
		flasher.pids->killall();
	}catch(std::exception&exc){
		log_exception(exc,"failed to stop remaining tasks");
	}
	flasher.pids=nullptr;
	chdir("/");
	if(!flasher.folder.empty())try{umount_img();}catch(...){}
	if(flasher.netblk)try{
		log_info("stopping netblk pid {}",flasher.netblk->pid);
		flasher.netblk->kill_wait();
	}catch(std::exception&exc){
		log_exception(exc,"failed to stop netblk");
	}
	if(!flasher.folder.empty()){
		auto mnt_dir=path_join(flasher.folder,"mnt");
		auto bin_dir=path_join(flasher.folder,"bin");
		unlink(path_join(bin_dir,"flasher").c_str());
		unlink(path_join(bin_dir,"netblk").c_str());
		unlink(path_join(bin_dir,"helper").c_str());
		rmdir(mnt_dir.c_str());
		rmdir(bin_dir.c_str());
		rmdir(flasher.folder.c_str());
	}
	flasher.netblk=nullptr;
}
