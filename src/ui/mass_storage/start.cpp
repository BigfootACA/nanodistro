#include<dirent.h>
#include<sys/mount.h>
#include"error.h"
#include"internal.h"
#include"fs-utils.h"
#include"str-utils.h"
#include"path-utils.h"
#include"modules.h"
#include"configs.h"
#include"process.h"

static void xmkdir(const std::string&path,mode_t mode=0755){
	log_debug("creating directory {}",path);
	if(mkdir(path.c_str(),mode)<0)
		throw ErrnoError("failed to create {}",path);
}

static void xsymlink(const std::string&src,const std::string&dest){
	log_debug("symlink {} to {}",src,dest);
	if(symlink(src.c_str(),dest.c_str())<0)
		throw ErrnoError("failed to symlink {} to {}",src,dest);
}

static std::list<std::string>apply_uas(mass_storage_config*cfg,const std::string&gadget){
	std::list<std::string>rfuncs{};
	module_load("usbfunc:tcm");
	module_load("target_core_pscsi");
	module_load("target_core_iblock");
	if(!cfg->multi_lun)
		throw RuntimeError("USB Attached SCSI requires multi-lun support");
	if(cfg->readonly)
		throw RuntimeError("USB Attached SCSI cannot be read-only");
	auto func=path_join(path_join(gadget,"functions"),"tcm.target");
	xmkdir(func);
	std::string target="/sys/kernel/config/target";
	auto pscsi=path_join(target,"core/pscsi_0");
	auto iblock=path_join(target,"core/iblock_0");
	if(!fs_exists(pscsi)&&cfg->scsi_direct)xmkdir(pscsi);
	if(!fs_exists(iblock))xmkdir(iblock);
	auto naa="naa."+cfg->str_serial_num;
	auto fabric=path_join(target,"usb_gadget");
	auto naap=path_join(fabric,naa);
	auto tpgt=path_join(naap,"tpgt_1");
	xmkdir(fabric);
	xmkdir(naap);
	xmkdir(tpgt);
	int lun=0;
	for(auto&disk:cfg->disks){
		auto path=path_join("/dev",disk);
		auto bdev=std::format("/sys/class/block/{}/device",disk);
		bool is_scsi=fs_exists(path_join(bdev,"scsi_device"));
		bool is_direct=is_scsi&&cfg->scsi_direct;
		int scsi_host=-1,scsi_chan=-1,scsi_tgt=-1,scsi_lun=-1;
		if(is_direct)try{
			auto scsi_id=path_basename(fs_readlink(bdev));
			auto scsi_ids=str_split(str_trim_to(scsi_id),':');
			if(scsi_ids.size()==4){
				scsi_host=std::stoi(scsi_ids[0]);
				scsi_chan=std::stoi(scsi_ids[1]);
				scsi_tgt=std::stoi(scsi_ids[2]);
				scsi_lun=std::stoi(scsi_ids[3]);
				if(scsi_host<0||scsi_chan<0||scsi_tgt<0||scsi_lun<0)
					throw RuntimeError("parse SCSI ID {} failed",scsi_id);
				auto scsi_fid=std::format("{}:{}:{}:{}",scsi_host,scsi_chan,scsi_tgt,scsi_lun);
				log_info("disk {} is SCSI {}",disk,scsi_fid);
				if(!fs_exists(std::format("/sys/bus/scsi/devices/{}/block/{}",scsi_fid,disk)))
					throw RuntimeError("SCSI device {} cannot resolve to {}",scsi_fid,disk);
			}else throw RuntimeError("invalid SCSI ID {}",scsi_id);
		}catch(std::exception&exc){
			log_exception(exc,"invalid SCSI for disk {}, force to iblock",disk);
				is_direct=false;
		}
		auto blkpath=path_join(is_direct?pscsi:iblock,disk);
		log_info("add disk {} to scsi target lun {} use {}",disk,lun,is_direct?"pscsi":"iblock");
		xmkdir(blkpath);
		auto ctrl=path_join(blkpath,"control");
		if(!is_direct)fs_write_all(ctrl,std::format("udev_path={}\n",path));
		else fs_write_all(ctrl,std::format(
			"udev_path={},"
			"scsi_host_id={},"
			"scsi_channel_id={},"
			"scsi_target_id={},"
			"scsi_lun_id={}\n",
			path,scsi_host,scsi_chan,scsi_tgt,scsi_lun
		));
		fs_write_all(path_join(blkpath,"udev_path"),path+"\n");
		fs_write_numlf(path_join(blkpath,"enable"),1);
		fs_write_all(path_join(blkpath,"wwn/vendor_id"),"NANODIST\n");
		fs_write_all(path_join(blkpath,"wwn/product_id"),disk+"\n");
		auto lunpath=path_join(tpgt,"lun/lun_"+std::to_string(lun++));
		xmkdir(lunpath);
		xsymlink(blkpath,path_join(lunpath,"virtual_scsi_port"));
	}
	fs_write_all(path_join(tpgt,"nexus"),naa+"\n");
	fs_write_numlf(path_join(tpgt,"enable"),1);
	rfuncs.push_back("tcm.target");
	return rfuncs;
}

static std::list<std::string>apply_msc(mass_storage_config*cfg,const std::string&gadget){
	module_load("usbfunc:mass_storage");
	std::list<std::string>rfuncs{};
	if(cfg->scsi_direct)
		throw RuntimeError("SCSI direct access is not supported for mass storage");
	auto funcs=path_join(gadget,"functions");
	auto apply_lun=[&](const std::string&func,int lun,const std::string&dev){
		log_info("add disk {} to mass storage lun {}",dev,lun);
		auto path=path_join("/dev",dev);
		if(!fs_exists(path))throw RuntimeError("disk {} does not exist",dev);
		auto lund=path_join(func,"lun."+std::to_string(lun));
		if(lun!=0)xmkdir(lund);
		fs_write_all(path_join(lund,"inquiry_string"),dev+"\n");
		fs_write_numlf(path_join(lund,"ro"),cfg->readonly?1:0);
		fs_write_numlf(path_join(lund,"removable"),1);
		fs_write_numlf(path_join(lund,"cdrom"),0);
		fs_write_all(path_join(lund,"file"),path+"\n");
	};
	if(cfg->multi_lun){
		std::string fn="mass_storage.disk";
		auto func=path_join(funcs,fn);
		xmkdir(func);
		int lun=0;
		for(auto&disk:cfg->disks)
			apply_lun(func,lun++,disk);
		rfuncs.push_back(fn);
	}else for(auto&disk:cfg->disks){
		std::string fn="mass_storage."+disk;
		auto func=path_join(funcs,fn);
		xmkdir(func);
		apply_lun(func,0,disk);
		rfuncs.push_back(fn);
	}
	return rfuncs;
}

void mass_storage_config::apply(){
	if(!initialized)
		throw RuntimeError("mass storage config is not initialized");
	if(disks.empty())throw RuntimeError("no disks to mount");
	if(udc.empty())throw RuntimeError("no UDC selected");
	module_load("libcomposite");
	call_hook("on-pre-start");
	std::string cfs="/sys/kernel/config/usb_gadget";
	if(!fs_exists(cfs))throw RuntimeError("usb gadget configfs is not available");
	clean();
	auto gadget=path_join(cfs,"nanodistro");
	mkdir(gadget.c_str(),0755);
	fs_write_all(path_join(gadget,"idVendor"),"0x1D6B\n"); /* Linux Foundation */
	fs_write_all(path_join(gadget,"idProduct"),"0x0104\n"); /* Multifunction Composite Gadget */
	auto str=path_join(path_join(gadget,"strings"),"0x409");
	xmkdir(str);
	fs_write_all(path_join(str,"manufacturer"),"ClassFun\n");
	fs_write_all(path_join(str,"product"),"NanoDistro\n");
	fs_write_all(path_join(str,"serialnumber"),str_serial_num+"\n");
	auto cfg=path_join(path_join(gadget,"configs"),"a.1");
	xmkdir(cfg);
	auto cfgstr=path_join(path_join(cfg,"strings"),"0x409");
	xmkdir(cfgstr);
	fs_write_all(path_join(cfgstr,"configuration"),"Mass Storage\n");
	auto os_desc=path_join(gadget,"os_desc");
	if(fs_exists(os_desc)){
		fs_write_all(path_join(os_desc,"use"),"1\n");
		fs_write_all(path_join(os_desc,"b_vendor_code"),"0x1\n");
		fs_write_all(path_join(os_desc,"qw_sign"),"MSFT100\n");
		auto ocfg=path_join(os_desc,"a.1");
		if(!fs_exists(ocfg))xsymlink(cfg,ocfg);
	}
	call_hook("on-start-storage");
	std::list<std::string>rfuncs{};
	switch(mode){
		case STORAGE_MODE_UAS:rfuncs=apply_uas(this,gadget);break;
		case STORAGE_MODE_MASS:rfuncs=apply_msc(this,gadget);break;
		default:throw RuntimeError("unknown storage mode {}",(int)mode);
	}
	if(rfuncs.empty())throw RuntimeError("no functions created for mass storage");
	auto funcs=path_join(gadget,"functions");
	for(auto&func:rfuncs)xsymlink(
		path_join(funcs,func),
		path_join(cfg,func)
	);
	call_hook("on-pre-start-udc");
	log_info("enable usb mass storage with {}",udc);
	fs_write_all(path_join(gadget,"UDC"),udc+"\n");
	call_hook("on-post-start");
}

void mass_storage_config::call_hook(const std::string&hook){
	std::vector<std::string>hooks{};
	if(auto v=config["nanodistro"]["gadget"][hook]){
		if(v.IsSequence())
			for(auto h:v)hooks.push_back(h.as<std::string>());
		if(v.IsScalar())
			hooks.push_back(v.as<std::string>());
	}
	if(hooks.empty())return;
	for(auto&h:hooks){
		log_info("calling gadget {} hook {}",hook,h);
		process p{};
		p.set_command(h);
		p.start();
		p.wait();
	}
}
