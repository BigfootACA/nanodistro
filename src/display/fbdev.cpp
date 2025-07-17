#include<fcntl.h>
#include<dirent.h>
#include<thread>
#include<semaphore>
#include<sys/mman.h>
#include<sys/ioctl.h>
#include<sys/prctl.h>
#include<linux/fb.h>
#include"internal.h"
#include"log.h"
#include"fs-utils.h"
#include"std-utils.h"
#include"path-utils.h"
#include"readable.h"
#include"cleanup.h"
#include"error.h"
#include"gui.h"

#ifndef PR_SET_VMA
#define PR_SET_VMA 0x53564d41
#endif
#ifndef PR_SET_VMA_ANON_NAME
#define PR_SET_VMA_ANON_NAME 0
#endif

class fbdev_display{
	public:
		~fbdev_display();
		void find_fbdev();
		bool try_fbdev(int i);
		void open_fbdev();
		void init_fbdev();
		void init_display();
		void refresh_thread();
		void flush_cb(lv_display_t*disp,const lv_area_t*area,uint8_t*color_p);
		lv_display_t*disp;
		std::string dev{};
		fb_var_screeninfo vinfo{};
		fb_fix_screeninfo finfo{};
		void*draw_buf=nullptr;
		void*mem=nullptr;
		uint32_t dpi_default=LV_DPI_DEF;
		int fb_fd=-1;
		std::binary_semaphore flush{0};
		bool disp_deleting=false;
		std::thread thread{};
};

class display_instance_fbdev:public display_instance{
	public:
		fbdev_display*fbdev=nullptr;
		void fill_color(lv_color_t color)override;
};

class display_backend_fbdev:public display_backend{
	public:
		std::shared_ptr<display_instance>init(const YAML::Node&cfg)override;
};

std::shared_ptr<display_backend>display_backend_create_fbdev(){
	return std::make_shared<display_backend_fbdev>();
}

void display_instance_fbdev::fill_color(lv_color_t color){
	if(!fbdev)return;
	#if LV_COLOR_DEPTH == 32
	lv_color32_t c{};
	#elif LV_COLOR_DEPTH == 16
	lv_color16_t c{};
	#else
	lv_color_t c{};
	#endif
	c.red=color.red;
	c.green=color.green;
	c.blue=color.blue;
	if(!fbdev->mem)return;
	auto sz=fbdev->finfo.smem_len;
	if(c.red==c.green&&c.red==c.blue)
		memset(fbdev->mem,c.red,sz);
	else for(size_t i=0;i<align_down(sz,sizeof(c));i+=sizeof(c))
		memcpy((uint8_t*)fbdev->mem+i,&c,sizeof(c));
}

std::shared_ptr<display_instance>display_backend_fbdev::init(const YAML::Node&cfg){
	auto fbdev=new fbdev_display();
	object_cleanup<fbdev_display*>cleanup(fbdev);
	if(auto v=cfg["dev"])
		fbdev->dev=v.as<std::string>();
	if(fbdev->dev.empty())fbdev->find_fbdev();
	else fbdev->open_fbdev();
	fbdev->init_fbdev();
	fbdev->init_display();
	if(auto v=cfg["dpi"])
		lv_display_set_dpi(fbdev->disp,v.as<int>());
	cleanup.take();
	auto ins=std::make_shared<display_instance_fbdev>();
	ins->fbdev=fbdev;
	ins->disp=fbdev->disp;
	return ins;
}

fbdev_display::~fbdev_display(){
	if(mem&&mem!=MAP_FAILED){
		munmap(mem,finfo.smem_len);
		mem=nullptr;
	}
	if(fb_fd>0){
		close(fb_fd);
		fb_fd=-1;
	}
	if(disp&&!disp_deleting)
		lv_display_delete(disp);
	flush.release();
	if(thread.joinable())
		thread.join();
	display_deinit_console();
}

void fbdev_display::refresh_thread(){
	prctl(PR_SET_NAME,"fbdev-refresh");
	while(fb_fd>0){
		flush.acquire();
		ioctl(fb_fd,FBIOPAN_DISPLAY,&vinfo);
	}
}

bool fbdev_display::try_fbdev(int i){
	if(!fs_exists(std::format("/sys/class/graphics/fb{}/dev",i)))return false;
	std::vector<std::string>devs{
		std::format("/dev/fb{}",i),
		std::format("/dev/graphics/fb{}",i),
	};
	for(auto&dev:devs)if(fs_exists(dev))this->dev=dev;
	if(dev.empty())return false;
	open_fbdev();
	std::string id(finfo.id,strnlen(finfo.id,sizeof(finfo.id)));
	if(id=="Virtual FB")return false;
	log_info("found fbdev {} with {}",dev,id);
	return true;
}

void fbdev_display::find_fbdev(){
	if(!dev.empty()||fb_fd>0)return;
	for(int i=0;i<UINT8_MAX;i++){
		bool accept=false;
		try{
			accept=try_fbdev(i);
		}catch(const std::exception&exc){
			log_exception(exc,"failed to open fbdev {}",i);
		}
		if(accept)return;
		if(fb_fd>0)close(fb_fd);
		fb_fd=-1,dev.clear();
		memset(&finfo,0,sizeof(finfo));
		memset(&vinfo,0,sizeof(vinfo));
	}
	throw RuntimeError("no available framebuffer device found");
}

void fbdev_display::open_fbdev(){
	fb_fd=open(dev.c_str(),O_RDWR|O_CLOEXEC);
	if(fb_fd<0)throw ErrnoError("failed to open framebuffer device {}",dev);
	log_info("open fbdev {} as {}",dev,fb_fd);
	xioctl(fb_fd,FBIOGET_FSCREENINFO,&finfo);
	xioctl(fb_fd,FBIOGET_VSCREENINFO,&vinfo);
	log_info("fbdev {} resolution {}x{} {}bpp",dev,vinfo.xres,vinfo.yres,vinfo.bits_per_pixel);
}

void fbdev_display::init_fbdev(){
	mem=mmap(0,finfo.smem_len,PROT_READ|PROT_WRITE,MAP_SHARED,fb_fd,0);
	if(!mem||mem==MAP_FAILED)throw ErrnoError("failed to map framebuffer device to memory");
	auto name=std::format("fbdev-{}",path_basename(dev));
	prctl(PR_SET_VMA,PR_SET_VMA_ANON_NAME,(uintptr_t)mem,finfo.smem_len,name.c_str());
	log_info("mapped fbdev at 0x{:x} size {}",(uintptr_t)mem,size_string_float(finfo.smem_len));
	memset(mem,0,finfo.smem_len);
	ioctl(fb_fd,FBIOPAN_DISPLAY,&vinfo);
	ioctl(fb_fd,FBIOBLANK,FB_BLANK_UNBLANK);
	thread=std::thread(std::bind(&fbdev_display::refresh_thread,this));
	display_setup_console();
}

void fbdev_display::flush_cb(lv_display_t*disp,const lv_area_t*area,uint8_t*color_p){
	if(
		gui_pause||area->x2<0||area->y2<0||
		area->x1>(int32_t)vinfo.xres-1||
		area->y1>(int32_t)vinfo.yres-1
	){
		lv_display_flush_ready(disp);
		return;
	}
	auto w=lv_area_get_width(area);
	auto cf=lv_display_get_color_format(disp);
	int32_t px_size=lv_color_format_get_size(cf);
	int32_t xoff=area->x1+vinfo.xoffset;
	int32_t yoff=area->y1+vinfo.yoffset;
	int32_t fb_pos=xoff*px_size+yoff*finfo.line_length;
	int32_t color_pos=area->x1*px_size+area->y1*disp->hor_res*px_size;
	for(int32_t y=area->y1;y<=area->y2;y++){
		auto fbp=(uint8_t*)mem;
		memcpy(&fbp[fb_pos],&color_p[color_pos],w*px_size);
		fb_pos+=finfo.line_length;
		color_pos+=disp->hor_res*px_size;
	}
	flush.release();
	lv_display_flush_ready(disp);
}

void fbdev_display::init_display(){
	if(disp)return;
	disp=lv_display_create(vinfo.xres,vinfo.yres);
	if(!disp)throw RuntimeError("failed to create display");
	lv_display_set_driver_data(disp,this);
	lv_display_set_flush_cb(disp,[](auto d,auto a,auto c){
		auto fbdev=(fbdev_display*)lv_display_get_driver_data(d);
		if(!fbdev||d!=fbdev->disp)return;
		fbdev->flush_cb(d,a,c);
	});
	lv_display_add_event_cb(disp,[](auto e){
		auto fbdev=(fbdev_display*)lv_event_get_user_data(e);
		if(!fbdev)return;
		fbdev->disp_deleting=true;
		delete fbdev;
	},LV_EVENT_DELETE,this);
	lv_color_format_t fmt;
	switch(vinfo.bits_per_pixel){
		case 16:fmt=LV_COLOR_FORMAT_RGB565;break;
		case 24:fmt=LV_COLOR_FORMAT_RGB888;break;
		case 32:fmt=LV_COLOR_FORMAT_XRGB8888;break;
		default:throw RuntimeError("{}bpp not supported",vinfo.bits_per_pixel);
	}
	lv_display_set_color_format(disp,fmt);
	auto draw_buf_size=vinfo.xres*vinfo.yres*(vinfo.bits_per_pixel>>3);
	if(!(draw_buf=malloc(draw_buf_size*2)))
		throw ErrnoError("failed to allocate display buffer");
	log_info(
		"allocated display buffer at 0x{:x} size {}",
		(uintptr_t)draw_buf,size_string_float(draw_buf_size*2)
	);
	auto buf1=draw_buf;
	auto buf2=(uint8_t*)draw_buf+draw_buf_size;
	lv_display_set_buffers(disp,buf1,buf2,draw_buf_size,LV_DISPLAY_RENDER_MODE_DIRECT);
	auto vdpi=dpi_default;
	if(vinfo.width>0)
		vdpi=div_round_up(vinfo.xres*254,vinfo.width*10);
	lv_display_set_dpi(disp,vdpi);
}
