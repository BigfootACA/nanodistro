#include"internal.h"
#include"path-utils.h"
#include"str-utils.h"
#include"fs-utils.h"
#include"cleanup.h"
#include"error.h"
#include"log.h"
#include<fcntl.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<sys/sysmacros.h>
#include<linux/fs.h>

static void set_if(std::map<std::string,std::string>&map,const std::string&key,const std::string&val){
	if(!val.empty())map[key]=val;
}

static std::string get_driver(const std::string&path){
	if(path.empty())return "";
	auto p=path_join(path,"driver");
	if(!fs_is_link(p))return "";
	auto driver=fs_readlink(path);
	return path_basename(driver);
}

static std::string get_dev_driver(const std::string&path){
	return get_driver(path_join(path,"device"));
}

static void load_disk_pcie(const std::shared_ptr<disk_item>&item,const std::string&pcie){
	set_if(item->info,"PCIE_SLOT",path_basename(fs_readlink(path_join(pcie,"device"))));
	set_if(item->info,"PCIE_LANE_WIDTH",fs_simple_read(pcie,"device/current_link_width"));
	set_if(item->info,"PCIE_LANE_BAND",fs_simple_read(pcie,"device/current_link_speed"));
	uint64_t num_band=0,num_speed=0;
	std::string band=item->info["PCIE_LANE_BAND"],gen{};
	if(band.starts_with("2.5 GT/s"))gen="1.0",num_band=2500,num_speed=250;
	if(band.starts_with("5.0 GT/s"))gen="2.0",num_band=5000,num_speed=500;
	if(band.starts_with("8.0 GT/s"))gen="3.0",num_band=8000,num_speed=985;
	if(band.starts_with("16.0 GT/s"))gen="4.0",num_band=16000,num_speed=1969;
	if(band.starts_with("32.0 GT/s"))gen="5.0",num_band=32000,num_speed=3938;
	if(band.starts_with("64.0 GT/s"))gen="6.0",num_band=64000,num_speed=7563;
	if(band.starts_with("128.0 GT/s"))gen="7.0",num_band=128000,num_speed=15125;
	set_if(item->info,"PCIE_GEN",gen);
	if(num_band>0&&num_speed>0){	
		auto lanes=std::stoi(item->info["PCIE_LANE_WIDTH"]);
		item->info_num["PCIE_LANE_WIDTH"]=lanes;
		item->info_num["PCIE_LANE_BAND"]=num_band*1000000;
		item->info_num["PCIE_LANE_SPEED"]=num_speed*1024*1024;
		item->info_num["PCIE_TOTAL_BAND"]=lanes*num_band*1000000;
		item->info_num["PCIE_TOTAL_SPEED"]=lanes*num_speed*1024*1024;
	}
}

static void load_disk_usb(const std::shared_ptr<disk_item>&item,const std::string&usb){
	set_if(item->info,"USB_PORT",path_basename(usb));
	set_if(item->info,"USB_SPEED",fs_simple_read(usb,"speed"));
	set_if(item->info,"USB_VERSION",fs_simple_read(usb,"version"));
	set_if(item->info,"USB_VENDOR_ID",fs_simple_read(usb,"idVendor"));
	set_if(item->info,"USB_PRODUCT_ID",fs_simple_read(usb,"idProduct"));
	set_if(item->info,"USB_SERIAL",fs_simple_read(usb,"serial"));
	set_if(item->info,"USB_MANUFACTURER",fs_simple_read(usb,"manufacturer"));
	set_if(item->info,"USB_PRODUCT",fs_simple_read(usb,"product"));
	auto speed=std::stof(item->info["USB_SPEED"]);
	item->info_num["USB_SPEED"]=speed*1024*1024;
}

static void load_disk_nvme(const std::shared_ptr<disk_item>&item){
	set_if(item->info,"MODEL",fs_simple_read(item->path_sysfs,"device/model"));
	set_if(item->info,"WWID",fs_simple_read(item->path_sysfs,"wwid"));
	set_if(item->info,"AREA",fs_simple_read(item->path_sysfs,"nsid"));
	if(auto ns=fs_simple_read(item->path_sysfs,"nsid");!ns.empty())
		item->info["AREA"]=std::format("NS{}",ns);
	auto rotational=fs_simple_read(item->path_sysfs,"queue/rotational");
	if(rotational=="0")item->info["MEDIA"]="SSD";
	else if(rotational=="1")item->info["MEDIA"]="HDD";
	else item->info["MEDIA"]="Disk";
	item->info["BUS"]="PCIe";
	item->info["PROTOCOL"]="NVMe";
	load_disk_pcie(item,path_join(item->path_sysfs,"device"));
}

static void load_disk_scsi(const std::shared_ptr<disk_item>&item){
	set_if(item->info,"VENDOR",fs_simple_read(item->path_sysfs,"device/vendor"));
	set_if(item->info,"MODEL",fs_simple_read(item->path_sysfs,"device/model"));
	set_if(item->info,"REVISION",fs_simple_read(item->path_sysfs,"device/rev"));
	set_if(item->info,"WWID",fs_simple_read(item->path_sysfs,"device/wwid"));
	auto scsi_id=fs_readlink(path_join(item->path_sysfs,"device"));
	auto scsi_parts=str_split(path_basename(scsi_id),':');
	if(scsi_parts.size()==4){
		item->info["SCSI_HOST"]=scsi_parts[0];
		item->info["SCSI_CHANNEL"]=scsi_parts[1];
		item->info["SCSI_ID"]=scsi_parts[2];
		item->info["SCSI_LUN"]=scsi_parts[3];
		item->info["AREA"]=std::format("LUN{}",scsi_parts[3]);
	}
	auto driver=get_dev_driver(item->path_sysfs);
	if(driver=="sd"){
		auto rotational=fs_simple_read(item->path_sysfs,"queue/rotational");
		if(rotational=="0")item->info["MEDIA"]="SSD";
		else if(rotational=="1")item->info["MEDIA"]="HDD";
		else item->info["MEDIA"]="Disk";
		item->info["MEDIA"]="Disk";
	}
	if(driver=="sr")item->info["MEDIA"]="CDROM";
	if(driver=="st")item->info["MEDIA"]="Tape";
	if(item->info.contains("SCSI_HOST")){
		auto scsi_host="/sys/class/scsi_host/host"+item->info["SCSI_HOST"];
		if(fs_exists(scsi_host+"/../ata_port")){
			auto driver=get_driver(scsi_host+"/../..");
			if(driver=="ahci"){
				item->info["BUS"]="SATA";
				item->info["PROTOCOL"]="SATA";
			}else{
				item->info["BUS"]="IDE";
				item->info["PROTOCOL"]="IDE";
			}
		}else if(fs_exists(scsi_host+"/../driver")){
			auto driver=get_driver(scsi_host+"/..");
			if(driver=="usb-storage"){
				item->info["BUS"]="USB";
				item->info["PROTOCOL"]="USB";
				load_disk_usb(item,scsi_host+"/../..");
			}
		}
	}
}

static void load_disk_mmc(const std::shared_ptr<disk_item>&item){
	set_if(item->info,"MODEL",fs_simple_read(item->path_sysfs,"device/name"));
	auto mmc_type=item->dev_uevent["MMC_TYPE"];
	if(mmc_type=="SD")item->info["MEDIA"]="sdcard";
	if(mmc_type=="SDIO")item->info["MEDIA"]="sdcard";
	if(mmc_type=="SDcombo")item->info["MEDIA"]="sdcard";
	if(mmc_type=="MMC")item->info["MEDIA"]="emmc";
	if(item->devname.starts_with("mmcblk")){
		auto pos=std::string::npos;
		if(pos==std::string::npos)
			pos=item->devname.find("boot");
		if(pos==std::string::npos)
			pos=item->devname.find("rpmb");
		if(pos==std::string::npos)
			pos=item->devname.find("gp");
		if(pos!=std::string::npos)
			item->info["AREA"]=item->devname.substr(pos);
	}
}

static void load_disk_mtd(const std::shared_ptr<disk_item>&item){
	set_if(item->info,"AREA",fs_simple_read(item->path_sysfs,"device/name"));
	set_if(item->info,"MEDIA",fs_simple_read(item->path_sysfs,"device/type"));
	auto device=fs_readlink(path_join(item->path_sysfs,"device/device"));
	if(!device.empty()){
		if(device.find("/spi")!=std::string::npos)
			item->info["BUS"]="SPI";
		if(device.find("/i2c")!=std::string::npos)
			item->info["BUS"]="I2C";
	}
}

std::shared_ptr<disk_item>ui_draw_choose_disk::load_disk(const std::string&dev){
	auto item=std::make_shared<disk_item>();
	item->devname=dev;
	item->path_sysfs=path_join("/sys/class/block",dev);

	/* skip partition */
	if(fs_exists(path_join(item->path_sysfs,"partition")))return nullptr;

	/* collect device path */
	auto dest=fs_readlink(item->path_sysfs);
	if(!dest.starts_with("../../devices/"))
		throw RuntimeError("invalid device path {} for {}",dest,item->path_sysfs);
	item->info["DEVPATH"]=dest.substr(5);

	/* skip virtual device*/
	if(item->info["DEVPATH"].starts_with("/devices/virtual/"))return nullptr;

	/* collect uevent */
	auto uevent_cont=fs_read_all(path_join(item->path_sysfs,"uevent"));
	auto uevent=parse_environ(uevent_cont);
	if(uevent.empty())throw RuntimeError("no uevent for {}",dev);
	item->uevent=uevent;

	/* collect uevent */
	auto dev_uevent_cont=fs_read_all(path_join(item->path_sysfs,"device/uevent"));
	auto dev_uevent=parse_environ(dev_uevent_cont);
	if(uevent.empty())throw RuntimeError("no device uevent for {}",dev);
	item->dev_uevent=dev_uevent;

	/* verify uevent */
	if(!uevent.contains("MAJOR")||!uevent.contains("MINOR"))
		throw RuntimeError("no major or minor for {}",dev);
	if(!uevent.contains("DEVNAME"))
		throw RuntimeError("no devname for {}",dev);
	if(uevent["DEVNAME"]!=dev)
		throw RuntimeError("devname mismatch {} != {}",uevent["DEVNAME"],dev);

	/* collect device node */
	item->dev=makedev(std::stoi(uevent["MAJOR"]),std::stoi(uevent["MINOR"]));
	if(item->dev==0)throw RuntimeError("invalid dev {}",dev);

	/* collect device node path and verify */
	struct stat st{};
	item->path_dev=path_join("/dev",dev);
	if(stat(item->path_dev.c_str(),&st)<0)
		throw ErrnoError("failed to stat {}",item->path_dev);
	if(!S_ISBLK(st.st_mode))throw RuntimeError(
		"{} is not a block device",item->path_dev
	);
	if(st.st_rdev!=item->dev)throw RuntimeError(
		"dev mismatch {} {:x} != {:x}",
		item->path_dev,st.st_rdev,item->dev
	);

	/* collect subsystem */
	auto subsys=fs_readlink(path_join(item->path_sysfs,"device/subsystem"));
	subsys=path_basename(subsys);
	item->info["SUBSYSTEM"]=subsys;

	/* collect disk size and sector size */
	int fd=open(item->path_dev.c_str(),O_RDONLY|O_CLOEXEC);
	if(fd<0)throw ErrnoError("failed to open {}",item->path_dev);
	cleanup_func closer(std::bind(&close,fd));
	item->info_num["DISKSIZE"]=xioctl_get(uint64_t,fd,BLKGETSIZE64);
	item->info_num["SECTOR_COUNT"]=xioctl_get(unsigned long,fd,BLKGETSIZE);
	item->info_num["SECTOR_SIZE"]=xioctl_get(int,fd,BLKSSZGET);
	closer.end();
	if(item->info_num["DISKSIZE"]==0)return nullptr;

	item->info["BUS"]=subsys;
	item->info["PROTOCOL"]=subsys;
	item->info["MEDIA"]=subsys;
	item->info["AREA"]="main";
	if(subsys=="nvme")load_disk_nvme(item);
	else if(subsys=="mmc")load_disk_mmc(item);
	else if(subsys=="scsi")load_disk_scsi(item);
	else if(subsys=="mtd")load_disk_mtd(item);
	
	if(item->info.contains("VENDOR")||item->info.contains("MODEL")){
		std::string product{};
		product+=item->info["VENDOR"];
		if(!product.empty())product+=' ';
		product+=item->info["MODEL"];
		if(!product.empty())
			item->info["PRODUCT"]=product;
	}

	data_disks.push_back(item);
	return item;
}

void ui_draw_choose_disk::load_disks(){
	std::string fail{},last_error="Unknown";
	size_t fails=0,success=0;
	std::lock_guard<std::mutex>lk(lock);
	try{
		std::string dir="/sys/class/block";
		DIR*d=opendir(dir.c_str());
		if(!d)throw ErrnoError("failed to open {}",dir);
		cleanup_func closer(std::bind(&closedir,d));
		data_disks.clear();
		while(auto e=readdir(d))try{
			if(e->d_type!=DT_LNK||e->d_name[0]=='.')continue;
			load_disk(e->d_name);
			success++;
		}catch(std::exception&exc){
			log_cur_exception("failed to retrieve disk {}",e->d_name);
			if(auto re=dynamic_cast<RuntimeErrorImpl*>(&exc))
				last_error=re->msg;
			else last_error=exc.what();
			fails++;
		}
		if(success==0&&fails>0)
			throw RuntimeError("all disks failed to load: {}",last_error);
		if(fails>0){
			lv_lock();
			msgbox_show(ssprintf(
				_("One or more disks failed to load: %s"),
				last_error.c_str()
			));
			lv_unlock();
		}

		lv_thread_call_func(std::bind(&ui_draw_choose_disk::draw_disk_items,this));
		return;
	}catch(std::exception&exc){
		log_exception(exc,"failed to retrieve disk list");
		if(auto re=dynamic_cast<RuntimeErrorImpl*>(&exc))
			fail=re->msg;
		else fail=exc.what();
	}
	lv_lock();
	set_spinner(false);
	msgbox_show(ssprintf(
		_("Failed to retrieve disk list: %s"),
		fail.c_str()
	));
	lv_unlock();
}
