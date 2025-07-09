#if defined(USE_LIB_LIBDRM)
#include<fcntl.h>
#include<dirent.h>
#include<cstring>
#include<poll.h>
#include<xf86drm.h>
#include<xf86drmMode.h>
#include<drm_fourcc.h>
#include<sys/mman.h>
#include<sys/prctl.h>
#include"std-utils.h"
#include"path-utils.h"
#include"internal.h"
#include"cleanup.h"
#include"error.h"
#include"gui.h"

#if LV_COLOR_DEPTH == 32
#define DRM_FOURCC DRM_FORMAT_XRGB8888
#elif LV_COLOR_DEPTH == 16
#define DRM_FOURCC DRM_FORMAT_RGB565
#else
#error LV_COLOR_DEPTH not supported
#endif

class drm_display;

struct drm_buffer{
	drm_display*drm=nullptr;
	size_t id=0;
	uint32_t handle=0;
	uint32_t pitch=0;
	uint64_t offset=0;
	size_t size=0;
	void*map=nullptr;
	uint32_t fb_handle=0;
	~drm_buffer();
	void allocate_dumb();
	void dmabuf_set_plane();
};

class drm_display{
	public:
		~drm_display();
		void add_prop(uint32_t type,const std::string&name,uint64_t val);
		inline void add_crtc_prop(const std::string&name,uint64_t val){
			return add_prop(DRM_MODE_OBJECT_CRTC,name,val);
		}
		inline void add_conn_prop(const std::string&name,uint64_t val){
			return add_prop(DRM_MODE_OBJECT_CONNECTOR,name,val);
		}
		inline void add_plane_prop(const std::string&name,uint64_t val){
			return add_prop(DRM_MODE_OBJECT_PLANE,name,val);
		}
		void init_display();
		void setup();
		void setup_props();
		void setup_connector();
		void setup_plane();
		void on_hotplug();
		void flush_cb(lv_display_t*disp,const lv_area_t*area,uint8_t*color_p);
		void flush_wait(lv_display_t*disp);
		void refr_start(lv_event_t*ev);
		static drm_display*find_create(const YAML::Node&cfg={});
		static drm_display*create(const std::string&dev,const YAML::Node&cfg={});
		lv_display_t*disp=nullptr;
		std::string card_dev{};
		void*draw_buf=nullptr;
		void*mem=nullptr;
		int card_fd=-1;
		bool disp_deleting=false;
		bool first_atomic=false;
		uint32_t conn_id=0,enc_id=0,crtc_id=0;
		uint32_t plane_id=0,crtc_idx=0;
		uint32_t width=0,height=0;
		uint32_t mm_width=0,mm_height=0;
		uint32_t fourcc=DRM_FOURCC;
		uint32_t blob_id=0;
		uint32_t dpi_default=LV_DPI_DEF;
		drmModeModeInfo mode{};
		drmModeAtomicReq*req=nullptr;
		drmEventContext drm_event_ctx{};
		drmModePlane*plane=nullptr;
		drmModeCrtc*crtc=nullptr;
		drmModeConnector*conn=nullptr;
		std::vector<drmModePropertyPtr>plane_props{};
		std::vector<drmModePropertyPtr>crtc_props{};
		std::vector<drmModePropertyPtr>conn_props{};
		std::vector<drm_buffer>buffers{};
		size_t active_buffer=SIZE_MAX;
};

class display_instance_drm:public display_instance{
	public:
		drm_display*drm=nullptr;
		void fill_color(lv_color_t color)override;
};

class display_backend_drm:public display_backend{
	public:
		std::shared_ptr<display_instance>init(const YAML::Node&cfg)override;
};

std::shared_ptr<display_backend>display_backend_create_drm(){
	return std::make_shared<display_backend_drm>();
}

void display_instance_drm::fill_color(lv_color_t color){
	if(!drm)return;
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
	uint32_t simple_fill=UINT32_MAX;
	if(c.red==c.green&&c.red==c.blue)
		simple_fill=c.red;
	for(auto&buf:drm->buffers){
		if(!buf.map)continue;
		if(simple_fill!=UINT32_MAX)
			memset(buf.map,simple_fill,buf.size);
		else for(size_t i=0;i<align_down(buf.size,sizeof(c));i+=sizeof(c))
			memcpy((uint8_t*)buf.map+i,&c,sizeof(c));
	}
}

std::shared_ptr<display_instance>display_backend_drm::init(const YAML::Node&cfg){
	std::string dev{};
	drm_display*drm=nullptr;
	if(auto v=cfg["dev"])
		dev=v.as<std::string>();
	if(!dev.empty())drm=drm_display::create(dev,cfg);
	else drm=drm_display::find_create(cfg);
	drm->init_display();
	if(auto v=cfg["dpi"])
		lv_display_set_dpi(drm->disp,v.as<int>());
	auto ins=std::make_shared<display_instance_drm>();
	ins->drm=drm;
	ins->disp=drm->disp;
	return ins;
}

drm_display*drm_display::create(const std::string&dev,const YAML::Node&cfg){
	auto drm=new drm_display;
	object_cleanup<drm_display*>cleanup(drm);
	drm->card_dev=dev;
	drm->setup();
	cleanup.take();
	return drm;
}

drm_display*drm_display::find_create(const YAML::Node&cfg){
	std::string dir="/dev/dri";
	auto d=opendir(dir.c_str());
	if(!d)throw RuntimeError("failed to open folder {}",dir);
	cleanup_func dir_free(std::bind(closedir,d));
	while(auto e=readdir(d)){
		std::string name=e->d_name;
		if(e->d_type!=DT_CHR)continue;
		if(!name.starts_with("card"))continue;
		auto dev=path_join(dir,name);
		try{
			return create(dev,cfg);
		}catch(const std::exception&exc){
			log_exception(exc,"skip device {}",dev);
		}
	}
	throw RuntimeError("no available drm device found");
}

static std::string fourcc_to_string(uint32_t fourcc){
	auto str=(const char*)&fourcc;
	return std::string(str,str+sizeof(fourcc));
}

drm_buffer::~drm_buffer(){
	if(fb_handle&&drm&&drm->card_fd>0){
		drmModeRmFB(drm->card_fd,fb_handle);
		fb_handle=0;
	}
	if(map&&map!=MAP_FAILED){
		munmap(map,size);
		map=nullptr;
	}
	if(handle>0&&drm&&drm->card_fd>0){
		drm_mode_destroy_dumb dreq{};
		dreq.handle=handle;
		drmIoctl(drm->card_fd,DRM_IOCTL_MODE_DESTROY_DUMB,&dreq);
		handle=0;
	}
}

drm_display::~drm_display(){
	if(disp&&!disp_deleting)
		lv_display_delete(disp);
	buffers.clear();
	for(auto&prop:plane_props)if(prop)drmModeFreeProperty(prop);
	for(auto&prop:crtc_props)if(prop)drmModeFreeProperty(prop);
	for(auto&prop:conn_props)if(prop)drmModeFreeProperty(prop);
	plane_props.clear();
	crtc_props.clear();
	conn_props.clear();
	if(crtc)drmModeFreeCrtc(crtc);
	if(conn)drmModeFreeConnector(conn);
	if(plane)drmModeFreePlane(plane);
	if(req)drmModeAtomicFree(req);
	crtc=nullptr;
	conn=nullptr;
	plane=nullptr;
	req=nullptr;
	if(card_fd>0){
		if(blob_id)drmModeDestroyPropertyBlob(card_fd,blob_id);
		close(card_fd);
		blob_id=0;
		card_fd=-1;
	}
	display_deinit_console();
}

void drm_display::init_display(){
	if(disp)return;
	disp=lv_display_create(width,height);
	if(!disp)throw RuntimeError("failed to create display");
	lv_display_set_driver_data(disp,this);
	lv_display_set_flush_cb(disp,[](auto d,auto a,auto c){
		auto drm=(drm_display*)lv_display_get_driver_data(d);
		if(!drm||d!=drm->disp)return;
		drm->flush_cb(d,a,c);
	});
	lv_display_set_flush_wait_cb(disp,[](auto d){
		auto drm=(drm_display*)lv_display_get_driver_data(d);
		if(!drm||d!=drm->disp)return;
		drm->flush_wait(d);
	});
	lv_display_add_event_cb(disp,[](auto e){
		auto drm=(drm_display*)lv_event_get_user_data(e);
		if(!drm)return;
		drm->disp_deleting=true;
		delete drm;
	},LV_EVENT_DELETE,this);
	lv_display_add_event_cb(disp,[](auto e){
		auto drm=(drm_display*)lv_event_get_user_data(e);
		if(drm)drm->refr_start(e);
	},LV_EVENT_REFR_START,this);
	size_t buf_size=std::min(buffers[1].size,buffers[0].size);
	lv_display_set_buffers(disp,buffers[1].map,buffers[0].map,buf_size,LV_DISPLAY_RENDER_MODE_DIRECT);	
	auto vdpi=dpi_default;
	if(mm_width>0)
		vdpi=div_round_up(width*25400,mm_width*1000);
	lv_display_set_dpi(disp,vdpi);
	display_setup_console();
}

void drm_display::setup_props(){
	auto setup_one=[&](auto name,uint32_t type,auto id,auto&v){
		auto props=drmModeObjectGetProperties(card_fd,id,type);
		if(!props)throw RuntimeError("drm get {} properties failed",name);
		cleanup_func props_free(std::bind(drmModeFreeObjectProperties,props));
		log_debug("found {} {} props",props->count_props,name);
		v.resize(props->count_props);
		for(uint32_t i=0;i<props->count_props;i++)
			v[i]=drmModeGetProperty(card_fd,props->props[i]);
	};
	setup_one("connector",DRM_MODE_OBJECT_CONNECTOR,conn_id,conn_props);
	setup_one("crtc",DRM_MODE_OBJECT_CRTC,crtc_id,crtc_props);
	setup_one("plane",DRM_MODE_OBJECT_PLANE,plane_id,plane_props);
}

void drm_display::add_prop(uint32_t type,const std::string&name,uint64_t value){
	int ret;
	uint32_t prop_id=0;
	auto find_props=[&]()->std::tuple<uint32_t,const char*,std::vector<drmModePropertyPtr>&> {
		switch(type){
			case DRM_MODE_OBJECT_CRTC:return {crtc_id,"crtc",crtc_props};
			case DRM_MODE_OBJECT_CONNECTOR:return {conn_id,"connector",conn_props};
			case DRM_MODE_OBJECT_PLANE:return {plane_id,"plane",plane_props};
			default:throw RuntimeError("unknown drm object type {}",type);
		}
	};
	auto[obj_id,obj_name,props]=find_props();
	for(auto&prop:props)if(prop&&std::string(prop->name)==name)prop_id=prop->prop_id;
	if(prop_id==0)throw RuntimeError("{} property {} not found",obj_name,name);
	ret=drmModeAtomicAddProperty(req,obj_id,prop_id,value);
	if(ret<0)throw RuntimeError("drm mode add {} property {} failed",obj_name,name);
}

void drm_buffer::dmabuf_set_plane(){
	uint32_t flags=DRM_MODE_PAGE_FLIP_EVENT|DRM_MODE_ATOMIC_NONBLOCK;
	if(!(drm->req=drmModeAtomicAlloc()))
		throw RuntimeError("drm mode atomic alloc failed");
	if(drm->first_atomic){
		drm->first_atomic=false;
		drm->add_conn_prop("CRTC_ID",drm->crtc_id);
		drm->add_crtc_prop("MODE_ID",drm->blob_id);
		drm->add_crtc_prop("ACTIVE",1);
		flags|=DRM_MODE_ATOMIC_ALLOW_MODESET;
	}
	drm->add_plane_prop("FB_ID",fb_handle);
	drm->add_plane_prop("CRTC_ID",drm->crtc_id);
	drm->add_plane_prop("SRC_X",0);
	drm->add_plane_prop("SRC_Y",0);
	drm->add_plane_prop("SRC_W",drm->width<<16);
	drm->add_plane_prop("SRC_H",drm->height<<16);
	drm->add_plane_prop("CRTC_X",0);
	drm->add_plane_prop("CRTC_Y",0);
	drm->add_plane_prop("CRTC_W",drm->width);
	drm->add_plane_prop("CRTC_H",drm->height);
	if(drmModeAtomicCommit(drm->card_fd,drm->req,flags,drm)<0)
		throw ErrnoError("drm mode atomic commit failed");
}

void drm_display::setup_plane(){
	drmModePlaneResPtr planes;
	if(!(planes=drmModeGetPlaneResources(card_fd)))
		throw RuntimeError("drm get plane resources failed");
	log_debug("found {} planes",planes->count_planes);
	bool plane_found=false;
	for(uint32_t i=0;i<planes->count_planes;i++)if(
		auto try_plane=drmModeGetPlane(card_fd,planes->planes[i])
	){
		cleanup_func plane_free(std::bind(drmModeFreePlane,try_plane));
		if(!(try_plane->possible_crtcs&(1<<crtc_idx)))continue;
		bool format_found=false;
		for(uint32_t j=0;j<try_plane->count_formats;j++)
			if(try_plane->formats[j]==fourcc)format_found=true;
		if(!format_found)continue;
		this->plane_id=try_plane->plane_id;
		this->plane=try_plane;
		plane_free.kill();
		log_debug("found plane {}",plane_id);
		plane_found=true;
	}
	if(!plane_found)throw RuntimeError("suitable plane not found");
}

void drm_display::setup_connector(){
	drmModeRes*res;
	if(!(res=drmModeGetResources(card_fd)))
		throw RuntimeError("drm get resources failed");
	cleanup_func res_free(std::bind(drmModeFreeResources,res));
	if(res->count_crtcs<=0)throw RuntimeError("no any crtc found");
	log_debug(
		"found {} connector {} encoder {} crtc",
		res->count_connectors,res->count_encoders,res->count_crtcs
	);
	drmModeConnector*conn=nullptr;
	for(int i=0;i<res->count_connectors;i++)if(
		auto try_conn=drmModeGetConnector(card_fd,res->connectors[i])
	){
		cleanup_func conn_free(std::bind(drmModeFreeConnector,try_conn));
		std::string connection{};
		switch(try_conn->connection){
			case DRM_MODE_CONNECTED:connection="connected";break;
			case DRM_MODE_DISCONNECTED:connection="disconnected";break;
			case DRM_MODE_UNKNOWNCONNECTION:connection="unknown";break;
			default:connection="unknown";break;
		}
		auto type=drmModeGetConnectorTypeName(try_conn->connector_type);
		log_info("connector{}: {} {}",try_conn->connector_id,type,connection);
		switch(try_conn->connector_type){
			case DRM_MODE_CONNECTOR_WRITEBACK:
				continue;
		}
		if(try_conn->connection!=DRM_MODE_CONNECTED)continue;
		if(try_conn->count_modes<=0)continue;
		conn_free.kill();
		conn=try_conn;
		break;
	}
	if(!conn)throw RuntimeError("suitable connector not found");
	conn_id=conn->connector_id;
	log_info("selected connector {}",conn_id);
	mm_width=conn->mmWidth;
	mm_height=conn->mmHeight;
	memcpy(&mode,&conn->modes[0],sizeof(drmModeModeInfo));
	if(drmModeCreatePropertyBlob(card_fd,&mode,sizeof(mode),&blob_id)<0)
		throw ErrnoError("create property blob failed");
	width=conn->modes[0].hdisplay;
	height=conn->modes[0].vdisplay;
	bool enc_found=false;
	auto apply_encoder=[&](auto try_enc){
		enc_id=try_enc->encoder_id;
		crtc_id=try_enc->crtc_id;
		log_debug("use encoder id {} crtc id {}",enc_id,crtc_id);
		enc_found=true;
	};
	for(int i=0;i<res->count_encoders;i++)if(
		auto try_enc=drmModeGetEncoder(card_fd,res->encoders[i])
	){
		cleanup_func enc_free(std::bind(drmModeFreeEncoder,try_enc));
		if(try_enc->encoder_id==conn->encoder_id){
			apply_encoder(try_enc);
			break;
		}
	}
	if(!enc_found)for(int i=0;i<conn->count_encoders;i++)if(
		auto try_enc=drmModeGetEncoder(card_fd,res->encoders[i])
	){
		cleanup_func enc_free(std::bind(drmModeFreeEncoder,try_enc));
		int crtc_id=-1;
		for(int crtc=0;crtc<res->count_crtcs;crtc++){
			uint32_t crtc_mask=1<<crtc;
			crtc_id=res->crtcs[crtc];
			log_debug(
				"enc id {} crtc{} id {} mask {:x} possible {:x}",
				try_enc->encoder_id,crtc,crtc_id,
				crtc_mask,try_enc->possible_crtcs
			);
			if(try_enc->possible_crtcs&crtc_mask)break;
		}
		if(crtc_id>0){
			apply_encoder(try_enc);
			break;
		}
	}
	if(!enc_found)throw RuntimeError("suitable encoder not found");
	bool crtc_found=false;
	for(uint32_t i=0;i<res->count_crtcs;i++)if(crtc_id==res->crtcs[i]){
		crtc_idx=i;
		crtc_found=true;
		break;
	}
	if(!crtc_found)throw RuntimeError("crtc not found");
	log_debug("crtc idx {}",crtc_idx);
}

void drm_display::setup(){
	int ret;
	if((card_fd=open(card_dev.c_str(),O_RDWR|O_CLOEXEC))<0)
		throw ErrnoError("failed to open drm device {}",card_dev);
	uint64_t has_dumb=0;
	if((ret=drmGetCap(card_fd,DRM_CAP_DUMB_BUFFER,&has_dumb))<0)
		throw ErrnoErrorWith(ret,"failed to get drm cap dumb buffer");
	if(has_dumb==0)throw RuntimeError("drm device {} does not support dumb buffer",card_dev);
	if((ret=drmSetClientCap(card_fd,DRM_CLIENT_CAP_ATOMIC,1))<0)
		throw ErrnoErrorWith(ret,"failed to set drm client atomic");
	setup_connector();
	setup_plane();
	if(!(plane=drmModeGetPlane(card_fd,plane_id)))
		throw ErrnoError("failed to get drm plane {}",plane_id);
	if(!(crtc=drmModeGetCrtc(card_fd,crtc_id)))
		throw ErrnoError("failed to get drm crtc {}",crtc_id);
	if(!(conn=drmModeGetConnector(card_fd,conn_id)))
		throw ErrnoError("failed to get drm connector {}",conn_id);
	setup_props();
	drm_event_ctx.version=DRM_EVENT_CONTEXT_VERSION;
	drm_event_ctx.page_flip_handler=[](auto,auto,auto,auto,auto d){
		auto drm=(drm_display*)d;
		if(!drm->req)return;
		drmModeAtomicFree(drm->req);
		drm->req=nullptr;
	};
	log_debug(
		"use plane {} connector {} crtc {}",
		plane_id,conn_id,crtc_id
	);
	log_info(
		"{}x{} ({}mm x {}mm) pixel format {}",
		width,height,mm_width,mm_height,
		fourcc_to_string(fourcc)
	);
	buffers.resize(2);
	for(size_t i=0;i<buffers.size();i++){
		buffers[i].drm=this;
		buffers[i].id=i;
		buffers[i].allocate_dumb();
	}
}

void drm_buffer::allocate_dumb(){
	int ret;
	uint32_t handles[4]{},pitches[4]{},offsets[4]{};
	drm_mode_create_dumb creq{};
	creq.width=drm->width;
	creq.height=drm->height;
	creq.bpp=LV_COLOR_DEPTH;
	ret=drmIoctl(drm->card_fd,DRM_IOCTL_MODE_CREATE_DUMB,&creq);
	if(ret<0)throw ErrnoError("drm ioctl create dumb failed");
	handle=creq.handle;
	pitch=creq.pitch;
	size=creq.size;
	drm_mode_map_dumb mreq{};
	mreq.handle=creq.handle;
	ret=drmIoctl(drm->card_fd,DRM_IOCTL_MODE_MAP_DUMB,&mreq);
	if(ret<0)throw ErrnoError("drm ioctl map dumb failed");
	offset=mreq.offset;
	map=mmap(nullptr,creq.size,PROT_READ|PROT_WRITE,MAP_SHARED,drm->card_fd,offset);
	if(!map||map==MAP_FAILED)throw ErrnoError("drm mmap buffer failed");
	auto name=std::format("drm-buffer-{}",id);
	prctl(PR_SET_VMA,PR_SET_VMA_ANON_NAME,(uintptr_t)map,creq.size,name.c_str());
	log_info("allocate buffer {} size 0x{:x} pitch {} offset 0x{:x}",id,size,pitch,offset);
	memset(map,0,creq.size);
	handles[0]=creq.handle;
	pitches[0]=creq.pitch;
	offsets[0]=0;
	ret=drmModeAddFB2(
		drm->card_fd,drm->width,drm->height,drm->fourcc,
		handles,pitches,offsets,&fb_handle,0
	);
	if(ret<0)throw ErrnoError("drm ioctl create dumb failed");
}

void drm_display::flush_wait(lv_display_t*disp){
	int ret;
	pollfd pfd{};
	pfd.fd=card_fd;
	pfd.events=POLLIN;
	while(req){
		ret=poll(&pfd,1,-1);
		if(ret<=0){
			if(ret<0&&errno==EINTR)continue;
			if(ret<0)log_error("poll failed: %s",strerror(errno));
			break;
		}
		drmHandleEvent(card_fd,&drm_event_ctx);
	}
}

void drm_display::flush_cb(lv_display_t*disp,const lv_area_t*,uint8_t*){
	if(!lv_display_flush_is_last(disp)||gui_pause)return;
	if(active_buffer==SIZE_MAX)throw RuntimeError("no active buffer");
	try{
		buffers[active_buffer].dmabuf_set_plane();
	}catch(const std::exception&exc){
		log_exception(exc,"failed to set active buffer");
	}
	active_buffer=SIZE_MAX;
}

void drm_display::refr_start(lv_event_t*ev){
	auto act_buf=lv_display_get_buf_active(disp);
	if(!act_buf||active_buffer!=SIZE_MAX)return;
	for(size_t i=0;i<buffers.size();i++){
		if(act_buf->unaligned_data==buffers[i].map){
			active_buffer=i;
			break;
		}
	}
}
#endif
