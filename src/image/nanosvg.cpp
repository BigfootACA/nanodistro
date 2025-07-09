#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include"nanosvg.h"
#include"nanosvgrast.h"
#include"cleanup.h"
#include"internal.h"
#include"error.h"

#define image_cache_draw_buf_handlers &(LV_GLOBAL_DEFAULT()->image_cache_draw_buf_handlers)

static std::string data_from_dsc(lv_image_decoder_dsc_t*dsc){
	if(!dsc||dsc->src_type!=LV_IMAGE_SRC_VARIABLE)return {};
	auto img_dsc=(const lv_image_dsc_t*)dsc->src;
	if(!img_dsc||!img_dsc->data)return {};
	return std::string(
		(const char*)img_dsc->data,
		(const char*)img_dsc->data+(size_t)img_dsc->data_size
	);
}

static bool is_svg(const std::string&data){
	return data.starts_with("<svg")||data.starts_with("<?xml");
}

static lv_result_t nanosvg_info(lv_image_decoder_t*decoder,lv_image_decoder_dsc_t*dsc,lv_image_header_t*header){
	if(!decoder||!dsc||!header)return LV_RESULT_INVALID;
	if(dsc->src_type!=LV_IMAGE_SRC_VARIABLE)return LV_RESULT_INVALID;
	if(!is_svg(data_from_dsc(dsc)))return LV_RESULT_INVALID;
	auto img_dsc=(const lv_image_dsc_t*)dsc->src;
	if(!img_dsc||!img_dsc->data)return {};
	header->cf=LV_COLOR_FORMAT_ARGB8888;
	header->w=img_dsc->header.w;
	header->h=img_dsc->header.h;
	header->stride=img_dsc->header.w*sizeof(lv_color_t);
	return LV_RESULT_OK;
}

static lv_draw_buf_t*svg_decode(const lv_image_dsc_t*src){
	auto fdpi=(float)lv_display_get_dpi(nullptr);
	std::string data=std::string(
		(const char*)src->data,
		(const char*)src->data+(size_t)src->data_size
	);
	auto m=nsvgParse(data.data(),"px",fdpi);
	if(!m)throw RuntimeError("failed to parse svg");
	cleanup_func cm(std::bind(nsvgDelete,m));
	auto r=nsvgCreateRasterizer();
	if(!r)throw RuntimeError("failed to create rasterizer");
	cleanup_func cr(std::bind(nsvgDeleteRasterizer,r));
	float scale=1.0f;
	if(src->header.w!=0&&src->header.h!=0)scale=std::min(
		(float)src->header.w/(float)m->width,
		(float)src->header.h/(float)m->height
	);
	size_t rw=m->width*scale,rh=m->height*scale;
	size_t rs=rw*sizeof(lv_color32_t);
	auto buf=lv_draw_buf_create_ex(
		image_cache_draw_buf_handlers,
		rw,rh,LV_COLOR_FORMAT_ARGB8888,rs
	);
	if(!buf)throw RuntimeError("failed to allocate svg image");
	cleanup_func cb(std::bind(lv_draw_buf_destroy,buf));
	if(buf->data_size!=rh*rs)throw RuntimeError("data size mismatch");
	if(!buf->data)throw RuntimeError("failed to allocate svg image");
	nsvgRasterize(r,m,0,0,scale,buf->data,rw,rh,rs);
	auto p=(lv_color32_t*)buf->data;
	for(size_t i=0;i<rw*rh;i++){
		auto old=p[i].blue;
		p[i].blue=p[i].red;
		p[i].red=old;
	}
	cb.kill();
	return buf;
}

static lv_result_t nanosvg_open(lv_image_decoder_t*decoder,lv_image_decoder_dsc_t*dsc){
	if(!decoder||!dsc)return LV_RESULT_INVALID;
	if(dsc->src_type!=LV_IMAGE_SRC_VARIABLE)return LV_RESULT_INVALID;
	auto img_dsc=(const lv_image_dsc_t*)dsc->src;
	if(!img_dsc||!img_dsc->data)return LV_RESULT_INVALID;
	if(!is_svg(data_from_dsc(dsc)))return LV_RESULT_INVALID;
	lv_draw_buf_t*buf=nullptr;
	try{
		auto decoded=svg_decode(img_dsc);
		cleanup_func cd(std::bind(lv_draw_buf_destroy,decoded));
		auto adjusted=lv_image_decoder_post_process(dsc,decoded);
		if(!adjusted)throw RuntimeError("failed to post process svg");
		cleanup_func ca(std::bind(lv_draw_buf_destroy,adjusted));
		if(decoded==adjusted)cd.kill();
		ca.kill();
		buf=adjusted,dsc->decoded=buf;
	}catch(std::exception&exc){
		log_exception(exc,"failed to decode svg");
		return LV_RESULT_INVALID;
	}
	if(dsc->args.no_cache||!lv_image_cache_is_enabled())return LV_RESULT_OK;
	lv_image_cache_data_t search_key;
	search_key.src_type=dsc->src_type;
	search_key.src=dsc->src;
	search_key.slot.size=dsc->decoded->data_size;
	auto entry=lv_image_decoder_add_to_cache(decoder,&search_key,dsc->decoded,NULL);
	if(!entry){
		lv_draw_buf_destroy(buf);
		dsc->decoded=nullptr;
		return LV_RESULT_INVALID;
	}
	dsc->cache_entry=entry;
	return LV_RESULT_OK;
}

static void nanosvg_close(lv_image_decoder_t*decoder, lv_image_decoder_dsc_t*dsc){
	if(dsc->args.no_cache||!lv_image_cache_is_enabled())
		lv_draw_buf_destroy((lv_draw_buf_t*)dsc->decoded);
}

static lv_image_decoder_t nanosvg_decoder{
	.info_cb=nanosvg_info,
	.open_cb=nanosvg_open,
	.close_cb=nanosvg_close,
	.name="nanosvg",
};

class image_backend_nanosvg:public image_backend{
	public:
		lv_image_decoder_t*init()override;
};

std::shared_ptr<image_backend>image_backend_create_nanosvg(){
	return std::make_shared<image_backend_nanosvg>();
}

lv_image_decoder_t*image_backend_nanosvg::init(){
	auto ret=lv_image_decoder_create();
	if(!ret)return nullptr;
	memcpy(ret,&nanosvg_decoder,sizeof(lv_image_decoder_t));
	return ret;
}
