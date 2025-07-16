#include<dirent.h>
#include<sys/mount.h>
#include<unistd.h>
#include"internal.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"path-utils.h"
#include"log.h"

static void xunlink(const std::string&path){
	if(!fs_exists(path))return;
	log_debug("removing {}",path);
	if(unlink(path.c_str())<0&&errno!=ENOENT)
		log_warning("failed to unlink {}: {}",path,strerror(errno));
}

static void xrmdir(const std::string&path){
	if(!fs_exists(path))return;
	log_debug("removing folder {}",path);
	if(rmdir(path.c_str())<0&&errno!=ENOENT)
		log_warning("failed to rmdir {}: {}",path,strerror(errno));
}

void gadget_clean(const std::string&func){
	if(func.empty())return;
	auto dir=path_join("/sys/kernel/config/usb_gadget",func);
	if(!fs_exists(dir))return;
	if(!str_trim_to(fs_read_all(path_join(dir,"UDC"))).empty())
		fs_write_all(path_join(dir,"UDC"),"\n");
	auto osds=path_join(dir,"os_desc");
	if(fs_exists(osds))fs_list_dir(osds,[&](auto name,auto dt){
		if(dt!=DT_LNK||name[0]=='.')return true;
		xunlink(path_join(osds,name));
		return true;
	});
	auto cfgs=path_join(dir,"configs");
	fs_list_dir(cfgs,[&](auto name,auto dt){
		if(dt!=DT_DIR||name[0]=='.')return true;
		auto cfg=path_join(cfgs,name);
		fs_list_dir(cfg,[&](auto name,auto dt){
			if(dt!=DT_LNK||name[0]=='.')return true;
			xunlink(path_join(cfg,name));
			return true;
		});
		auto cfgstrs=path_join(cfg,"strings");
		fs_list_dir(cfgstrs,[&](auto name,auto dt){
			if(dt!=DT_DIR||name[0]=='.')return true;
			xrmdir(path_join(cfgstrs,name));
			return true;
		});
		xrmdir(cfg);
		return true;
	});
	auto strs=path_join(dir,"strings");
	fs_list_dir(strs,[&](auto name,auto dt){
		if(dt!=DT_DIR||name[0]=='.')return true;
		xrmdir(path_join(strs,name));
		return true;
	});
	auto funcs=path_join(dir,"functions");
	fs_list_dir(funcs,[&](auto name,auto dt){
		if(dt!=DT_DIR||name[0]=='.')return true;
		auto path=path_join(funcs,name);
		if(name.starts_with("mass_storage."))fs_list_dir(path,[&](auto name,auto dt){
			if(name=="lun.0")return true;
			if(name.starts_with("lun."))
				xrmdir(path_join(path,name));
			return true;
		});
		xrmdir(path_join(funcs,name));
		return true;
	});
	xrmdir(dir);
}

void gadget_clean_all(){
	std::string dir="/sys/kernel/config/usb_gadget";
	if(!fs_exists(dir))return;
	fs_list_dir(dir,[&](const std::string&name,int dt){
		if(dt!=DT_DIR||name[0]=='.')return true;
		auto path=path_join(dir,name);
		if(!fs_exists(path_join(path,"functions")))return true;
		gadget_clean(name);
		return true;
	});
}

void mass_storage_config::clean(){
	std::string tgt="/sys/kernel/config/target";
	auto luns=path_join(tgt,"usb_gadget/naa.1234567890/tpgt_1/lun");
	if(fs_exists(luns))fs_list_dir(luns,[&](auto name,auto dt){
		if(dt!=DT_DIR||name[0]=='.')return true;
		if(!name.starts_with("lun_"))return true;
		auto path=path_join(luns,name);
		fs_list_dir(path,[&](auto name,auto dt){
			if(dt!=DT_LNK||name[0]=='.')return true;
			xunlink(path_join(path,name));
			return true;
		});
		xrmdir(path.c_str());
		return true;
	});
	xrmdir(path_join(tgt,"usb_gadget/naa.1234567890/tpgt_1").c_str());
	xrmdir(path_join(tgt,"usb_gadget/naa.1234567890").c_str());
	xrmdir(path_join(tgt,"usb_gadget").c_str());
	auto iblock=path_join(tgt,"core/iblock_0");
	auto pscsi=path_join(tgt,"core/pscsi_0");
	if(fs_exists(iblock))fs_list_dir(iblock,[&](auto name,auto dt){
		if(dt!=DT_DIR||name[0]=='.')return true;
		xrmdir(path_join(iblock,name).c_str());
		return true;
	});
	if(fs_exists(pscsi))fs_list_dir(pscsi,[&](auto name,auto dt){
		if(dt!=DT_DIR||name[0]=='.')return true;
		xrmdir(path_join(pscsi,name).c_str());
		return true;
	});
	xrmdir(iblock.c_str());
	xrmdir(pscsi.c_str());
	gadget_clean("nanodistro");
}
