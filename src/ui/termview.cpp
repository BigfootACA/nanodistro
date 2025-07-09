#include<cstring>
#include"termview.h"
#include"shl-llog.h"
#include"str-utils.h"
#include"libtsm.h"
#include"log.h"
#define LV_OBJX_NAME "lv_termview"

void lv_termview_resize(lv_obj_t*tv){
	if(!tv)return;
	uint32_t cols,rows;
	auto term=(lv_termview_t*)tv;
	if(!term||!term->screen||!term->vte)return;
	lv_obj_update_layout(tv);
	term->width=lv_obj_get_width(tv);
	term->height=lv_obj_get_height(tv);
	size_t size=term->height*term->width*4;
	if(size<=0)return;
	log_info("new size {}x{}",term->width,term->height);
	if(size!=term->mem_size){
		lv_color32_t*old=term->buffer;
		log_info("new buffer size {} bytes",size);
		if(!(term->buffer=(lv_color32_t*)malloc(size))){
			term->buffer=old;
			log_error("cannot allocate buffer for canvas");
			return;
		}
		term->mem_size=size;
		lv_canvas_set_buffer(
			tv,term->buffer,
			term->width,
			term->height,
			LV_COLOR_FORMAT_XRGB8888
		);
		if(old)free(old);
	}
	if(!term->cust_font){
		term->font_reg=lv_obj_get_style_text_font(
			tv,LV_PART_MAIN
		);
		term->font_bold=term->font_reg;
		term->font_ital=term->font_reg;
		term->font_bold_ital=term->font_reg;
	}
	if(!term->font_reg)return;
	if(term->glyph_height!=term->font_reg->line_height){
		term->glyph_height=term->font_reg->line_height;
		term->glyph_width=term->font_reg->line_height/2;
	}
	if(term->glyph_height<=0||term->glyph_width<=0){
		log_warning("invalid glyph size");
		return;
	}
	cols=term->width/term->glyph_width;
	rows=term->height/term->glyph_height;
	log_info("new terminal cols {} rows {}",cols,rows);
	if(cols!=term->cols||rows!=term->rows){
		term->cols=cols,term->rows=rows;
		tsm_screen_resize(term->screen,cols,rows);
		if(term->resize_cb)
			term->resize_cb(tv,cols,rows);
	}
	auto bg=lv_obj_get_style_bg_color(tv,0);
	lv_canvas_fill_bg(tv,bg,LV_OPA_COVER);
	term->max_sb=LV_MAX(
		term->max_sb,
		(uint32_t)(cols*rows*128)
	);
	tsm_screen_set_max_sb(
		term->screen,
		term->max_sb
	);
	term->age=0;
	lv_termview_update(tv);
}

struct tsm_draw_ctx{
	lv_layer_t layer{};
	lv_termview_t*term=nullptr;
	char buff[16384]{};
	size_t buff_pos=0;
	bool full_fill=false;
};

static int term_draw_cell(
	tsm_screen*screen,
	uint64_t,
	const uint32_t*cs,
	size_t len,
	uint32_t cw,
	uint32_t px,
	uint32_t py,
	const tsm_screen_attr*a,
	tsm_age_t age,
	void*data
){
	auto ctx=(tsm_draw_ctx*)data;
	lv_coord_t dw,dh,x,y;
	if(!ctx->term||screen!=ctx->term->screen||cw<=0)return 0;
	if(px>=ctx->term->cols||py>=ctx->term->rows)return 0;
	if(age&&ctx->term->age&&age<=ctx->term->age)return 0;
	x=px*ctx->term->glyph_width,y=py*ctx->term->glyph_height;
	dw=ctx->term->glyph_width*cw,dh=ctx->term->glyph_height;
	if(px==ctx->term->cols-1||dh+x>ctx->term->width)
		dw=ctx->term->width-x;
	if(py==ctx->term->rows-1||dh+y>ctx->term->height)
		dh=ctx->term->height-y;

	auto bg=lv_obj_get_style_bg_color((lv_obj_t*)ctx->term,0);
	if(!ctx->full_fill){
		ctx->full_fill=true;
		lv_area_t area{};
		area.x1=0,area.x2=ctx->term->width-1;
		area.y1=0,area.y2=ctx->term->height-1;
		lv_draw_fill_dsc_t fill{};
		lv_draw_fill_dsc_init(&fill);
		fill.color=bg;
		lv_draw_fill(&ctx->layer,&fill,&area);
	}

	lv_draw_fill_dsc_t rd;
	lv_draw_fill_dsc_init(&rd);
	auto fc=lv_color_make(a->fr,a->fg,a->fb);
	auto bc=lv_color_make(a->br,a->bg,a->bb);
	rd.color=a->inverse?fc:bc;
	lv_area_t area{};
	area.x1=x,area.x2=x+dw-1;
	area.y1=y,area.y2=y+dh-1;
	if(memcmp(&rd.color,&bg,sizeof(bg))!=0)
		lv_draw_fill(&ctx->layer,&rd,&area);
	if(len<=0)return 0;

	const lv_font_t*font;
	if(a->bold&&a->italic)font=ctx->term->font_bold_ital;
	else if(a->italic)font=ctx->term->font_ital;
	else if(a->bold)font=ctx->term->font_bold;
	else font=ctx->term->font_reg;
	if(!font)return 0;

	if(len==1){
		lv_draw_letter_dsc_t cd;
		lv_point_t p{x,y};
		p.x+=ctx->term->glyph_width/2;
		p.y+=ctx->term->glyph_height;
		lv_draw_letter_dsc_init(&cd);
		cd.color=a->inverse?bc:fc;
		cd.font=font;
		cd.unicode=cs[0];
		if(a->underline)cd.decor=LV_TEXT_DECOR_UNDERLINE;
		lv_draw_letter(&ctx->layer,&cd,&p);
	}else if(len>1){
		size_t off=0;
		auto ptr=ctx->buff+ctx->buff_pos;
		for(size_t i=0;i<len;i++){
			if(ctx->buff_pos+off+4>=sizeof(ctx->buff)-1)break;
			auto cnt=tsm_ucs4_to_utf8(cs[i],&ptr[off]);
			off+=cnt;
		}
		if(off>0){
			lv_draw_label_dsc_t ld;
			lv_area_t area{};
			area.x1=x,area.y1=y;			
			area.x1+=ctx->term->glyph_width/2;
			area.y1+=ctx->term->glyph_height;
			area.x2=area.x1+dw-1,area.y2=area.y1+dh-1;
			lv_draw_label_dsc_init(&ld);
			ld.color=a->inverse?bc:fc;
			ld.font=font;
			ld.text=ptr;
			if(a->underline)ld.decor=LV_TEXT_DECOR_UNDERLINE;
			lv_draw_label(&ctx->layer,&ld,&area);
		}
	}
	return 0;
}

static void log_cb(
	void*,
	const char*file,
	int line,
	const char*fn,
	const char*subs,
	unsigned int sev,
	const char*format,
	va_list args
){
	log_level lvl;
	switch(sev){
		case LLOG_DEBUG:lvl=LOG_DEBUG;break;
		case LLOG_INFO:lvl=LOG_INFO;break;
		case LLOG_NOTICE:lvl=LOG_INFO;break;
		case LLOG_WARNING:lvl=LOG_WARNING;break;
		case LLOG_ERROR:lvl=LOG_ERROR;break;
		case LLOG_CRITICAL:lvl=LOG_ERROR;break;
		case LLOG_ALERT:lvl=LOG_ERROR;break;
		case LLOG_FATAL:lvl=LOG_ERROR;break;
		default:lvl=LOG_DEBUG;
	}
	log_location loc{};
	loc.file=file;
	loc.line=line;
	loc.func=fn;
	loc.column=sev;
	auto data=vssprintf(format,args);
	log_print(lvl,data,loc);
}

static void write_cb(
	tsm_vte*vte,
	const char*u8,
	size_t len,
	void*data
){
	auto tv=(lv_obj_t*)data;
	auto term=(lv_termview_t*)data;
	if(!vte||!tv||!term||term->vte!=vte)return;
	if(term->write_cb)term->write_cb(tv,u8,len);
}

static void osc_cb(
	tsm_vte*vte,
	const char*u8,
	size_t len,
	void*data
){
	auto tv=(lv_obj_t*)data;
	auto term=(lv_termview_t*)tv;
	if(!vte||!tv||!term||term->vte!=vte)return;
	if(term->osc_cb)term->osc_cb(tv,u8,len);
}

tsm_screen*lv_termview_get_screen(lv_obj_t*tv){
	auto term=(lv_termview_t*)tv;
	return term->screen;
}

tsm_vte*lv_termview_get_vte(lv_obj_t*tv){
	auto term=(lv_termview_t*)tv;
	return term->vte;
}

void lv_termview_set_osc_cb(
	lv_obj_t*tv,
	termview_osc_cb cb
){
	auto term=(lv_termview_t*)tv;
	term->osc_cb=cb;
}

void lv_termview_set_write_cb(
	lv_obj_t*tv,
	termview_write_cb cb
){
	auto term=(lv_termview_t*)tv;
	term->write_cb=cb;
}

void lv_termview_set_resize_cb(
	lv_obj_t*tv,
	termview_resize_cb cb
){
	auto term=(lv_termview_t*)tv;
	term->resize_cb=cb;
}

void lv_termview_set_mods(
	lv_obj_t*tv,
	uint32_t mods
){
	auto term=(lv_termview_t*)tv;
	term->mods=mods;
}

void lv_termview_set_max_scroll_buffer_size(
	lv_obj_t*tv,
	uint32_t max_sb
){
	auto term=(lv_termview_t*)tv;
	term->max_sb=max_sb;
	tsm_screen_set_max_sb(term->screen,max_sb);
}

void lv_termview_set_font(lv_obj_t*tv,const lv_font_t*font){
	auto term=(lv_termview_t*)tv;
	if(!font||font->line_height<=0)return;
	term->glyph_height=font->line_height;
	term->glyph_width=font->line_height/2;
	term->font_reg=font;
	term->font_bold=font;
	term->font_ital=font;
	term->font_bold_ital=font;
	term->cust_font=true;
}

void lv_termview_set_font_regular(lv_obj_t*tv,lv_font_t*font){
	auto term=(lv_termview_t*)tv;
	if(!font||font->line_height<=0)return;
	term->glyph_height=font->line_height;
	term->glyph_width=font->line_height/2;
	term->font_reg=font;
	term->cust_font=true;
}

void lv_termview_set_font_bold(lv_obj_t*tv,lv_font_t*font){
	auto term=(lv_termview_t*)tv;
	if(!font||font->line_height<=0)return;
	term->font_bold=font;
	term->cust_font=true;
}

void lv_termview_set_font_italic(lv_obj_t*tv,lv_font_t*font){
	auto term=(lv_termview_t*)tv;
	if(!font||font->line_height<=0)return;
	term->font_ital=font;
	term->cust_font=true;
}

void lv_termview_set_font_bold_italic(lv_obj_t*tv,lv_font_t*font){
	auto term=(lv_termview_t*)tv;
	if(!font)return;
	term->font_bold_ital=font;
	term->cust_font=true;
}

void lv_termview_update(lv_obj_t*tv){
	auto term=(lv_termview_t*)tv;
	tsm_draw_ctx ctx{};
	ctx.term=term;
	if(!lv_obj_is_visible(tv))return;
	auto now=lv_tick_get();
	if(now-term->last_update<LV_DEF_REFR_PERIOD*2)return;
	term->last_update=now;
	lv_canvas_init_layer(tv,&ctx.layer);
	term->age=tsm_screen_draw(term->screen,term_draw_cell,&ctx);
	lv_canvas_finish_layer(tv,&ctx.layer);
}

uint32_t lv_termview_get_cols(lv_obj_t*tv){
	auto term=(lv_termview_t*)tv;
	return term->cols;
}

uint32_t lv_termview_get_rows(lv_obj_t*tv){
	auto term=(lv_termview_t*)tv;
	return term->rows;
}

void lv_termview_input(lv_obj_t*tv,const char*str){
	tsm_vte_input(
		lv_termview_get_vte(tv),
		str,strlen(str)
	);
	lv_termview_update(tv);
}

void lv_termview_line_prints(lv_obj_t*tv,const std::string&str){
	lv_termview_line_print(tv,str.c_str());
}

void lv_termview_line_print(lv_obj_t*tv,const char*str){
	auto term=(lv_termview_t*)tv;
	tsm_vte_input(term->vte,"\r",1);
	if(tsm_screen_get_cursor_x(term->screen)>0)
		tsm_vte_input(term->vte,"\n",1);
	tsm_vte_input(term->vte,str,strlen(str));
	tsm_vte_input(term->vte,"\r\n",2);
	lv_termview_update(tv);
}

static void lv_termview_event(lv_event_t*e){
	auto target=(lv_obj_t*)lv_event_get_target(e);
	auto term=(lv_termview_t*)target;
	switch(e->code){
		case LV_EVENT_PRESSING:{
			lv_point_t p;
			lv_indev_t*act=lv_indev_get_act();
			lv_indev_get_point(act,&p);
			if(term->drag_y_last==0)term->drag_y_last=p.y;
			else{
				bool up=true;
				lv_coord_t y=p.y-term->drag_y_last;
				if(y<0)y=-y,up=false;
				int ln=y/term->font_reg->line_height;
				if(ln>0){
					term->drag_y_last=p.y;
					if(up)tsm_screen_sb_up(term->screen,ln);
					else tsm_screen_sb_down(term->screen,ln);
					lv_termview_update(target);
				}
			}
		}break;
		case LV_EVENT_SIZE_CHANGED:lv_termview_resize(target);break;
		case LV_EVENT_RELEASED:term->drag_y_last=0;break;
		case LV_EVENT_REFR_REQUEST:lv_termview_update(target);break;
		default:;
	}
}

static void lv_termview_constructor(const lv_obj_class_t*,lv_obj_t*obj){
	lv_termview_t*ext=(lv_termview_t*)obj;
	lv_termview_resize(obj);
	if(tsm_screen_new(
		&ext->screen,
		NULL,NULL
	)!=0)log_error("create tsm screen failed");
	if(tsm_vte_new(
		&ext->vte,ext->screen,
		write_cb,obj,
		log_cb,NULL
	)<0)log_error("create vte screen failed");
	ext->cust_font=false;
	ext->glyph_height=0,ext->glyph_width=0;
	ext->cols=0,ext->rows=0,ext->age=0;
	ext->buffer=NULL,ext->mem_size=0;
	ext->mods=0,ext->drag_y_last=0;
	ext->resize_cb=NULL;
	ext->write_cb=NULL;
	ext->osc_cb=NULL;
	lv_obj_remove_flag(obj,LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_scrollbar_mode(obj,LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_flag(obj,LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_style_bg_color(obj,lv_color_black(),0);
	lv_obj_set_style_text_color(obj,lv_color_white(),0);
	tsm_vte_set_osc_cb(ext->vte,osc_cb,obj);
	lv_termview_resize(obj);
	lv_obj_add_event_cb(obj,lv_termview_event,LV_EVENT_ALL,NULL);
	log_info("terminal view created");
}

static void lv_termview_destructor(const lv_obj_class_t*,lv_obj_t*obj){
	lv_termview_t*term=(lv_termview_t *)obj;
	if(term->screen)tsm_screen_unref(term->screen);
	if(term->vte)tsm_vte_unref(term->vte);
	if(term->buffer)free(term->buffer);
	term->screen=NULL;
	term->vte=NULL;
	term->buffer=NULL;
}

const lv_obj_class_t lv_termview_class={
	.base_class = &lv_canvas_class,
	.constructor_cb = lv_termview_constructor,
	.destructor_cb = lv_termview_destructor,
	.name = "lv_termview",
	.width_def = LV_DPI_DEF * 2,
	.height_def = LV_DPI_DEF,
	.instance_size = sizeof(lv_termview_t),
};

lv_obj_t*lv_termview_create(lv_obj_t*parent){
	log_debug("begin");
	lv_obj_t*obj=lv_obj_class_create_obj(&lv_termview_class,parent);
	lv_obj_class_init_obj(obj);
	return obj;
}
