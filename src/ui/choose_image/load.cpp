#include"str-utils.h"
#include"internal.h"
#include"configs.h"
#include"request.h"
#include"error.h"
#include"log.h"

void ui_draw_choose_image::load_images(){
	std::string fail{};
	try{
		lv_lock();
		set_spinner(true);
		lv_label_set_text(lbl_loading,_("Loading manifest..."));
		lv_unlock();
		usleep(500000);

		std::string manifest_path{};
		if(auto v=config["nanodistro"]["server"]["manifest"])
			manifest_path=v.as<std::string>();
		if(manifest_path.empty())
			throw RuntimeError("manifest path is empty");

		url manifest(manifest_path);
		auto manifest_data=request_get_json(manifest);

		url devices_url;
		if(auto v=manifest_data["manifest"]["devices_url"])
			devices_url=manifest.relative(v.asString());
		else throw RuntimeError("devices_url not found in manifest");

		lv_lock();
		lv_label_set_text(lbl_loading,_("Loading devices..."));
		lv_unlock();
		usleep(200000);

		auto devices_data=request_get_json(devices_url);

		std::string device_id{};
		if(auto v=config["nanodistro"]["device"]["id"])
			device_id=v.as<std::string>();
		if(device_id.empty())
			throw RuntimeError("device id is not set");
		bool found=false;
		Json::Value device{};
		for(auto&d:devices_data["devices"]){
			if(d["id"].asString()!=device_id)continue;
			device=d,found=true;
			break;
		}
		if(!found)throw RuntimeError("device {} not found in devices",device_id);

		url images_url;
		if(auto v=device["images_url"])
			images_url=devices_url.relative(v.asString());
		else throw RuntimeError("images_url not found in device node");

		lv_lock();
		lv_label_set_text(lbl_loading,_("Loading images..."));
		lv_unlock();
		usleep(200000);

		auto images_data=request_get_json(images_url);

		if(!images_data["groups"].isArray())
			throw RuntimeError("images groups not found in images data");
		if(!images_data["images"].isArray())
			throw RuntimeError("images items not found in images data");

		this->images_url=images_url;
		data_images=images_data["images"];
		data_groups=images_data["groups"];

		usleep(200000);
		lv_thread_call_func(std::bind(&ui_draw_choose_image::populate_images,this));
		return;
	}catch(std::exception&exc){
		log_exception(exc,"failed to load images");
		if(auto re=dynamic_cast<RuntimeErrorImpl*>(&exc))
			fail=re->msg;
		else fail=exc.what();
	}
	lv_lock();
	set_spinner(false);
	msgbox_show(ssprintf(
		_("Failed to load images: %s"),
		fail.c_str()
	));
	lv_unlock();
}

void ui_draw_choose_image::load_cover(std::shared_ptr<data_item>item){
	if(!item||!item->img_cover)return;
	auto cover_url_s=item->data["cover_url"].asString();
	if(cover_url_s.empty())return;

	auto cover_url=images_url.relative(cover_url_s);
	item->img_data=request_get_data(cover_url);

	if(item->img_data.empty())return;
	item->img_dsc.data=(const uint8_t*)item->img_data.c_str();
	item->img_dsc.data_size=item->img_data.size();

	lv_lock();
	if(item->img_cover)lv_image_set_src(item->img_cover,&item->img_dsc);
	lv_unlock();
}

void ui_draw_choose_image::populate_images(){
	lv_group_remove_obj(btn_refresh);
	lv_group_remove_obj(btn_next);
	clean();

	for(auto&g:data_groups){
		auto item=std::make_shared<group_item>();
		item->id=g["id"].asString();
		item->data=g;
		groups.push_back(item);
		populate_group(item);
	}
	
	for(auto&g:data_images){
		auto item=std::make_shared<image_item>();
		item->id=g["id"].asString();
		item->data=g;
		images.push_back(item);
		populate_image(item);
	}

	auto grp=lv_group_get_default();
	lv_group_add_obj(grp,btn_refresh);
	lv_group_add_obj(grp,btn_next);
	set_spinner(false);
	lv_group_focus_obj(btn_refresh);

	lbl_group_placeholder=lv_label_create(lst_group);
	lv_obj_set_style_text_align(lbl_group_placeholder,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_size(lbl_group_placeholder,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_label_set_text(lbl_group_placeholder,_("No group found"));
	lv_obj_center(lbl_group_placeholder);
	lv_obj_set_hidden(lbl_group_placeholder,!groups.empty());
	lv_obj_set_style_layout(lst_group,groups.empty()?LV_LAYOUT_NONE:LV_LAYOUT_FLEX,0);

	lbl_image_placeholder=lv_label_create(lst_image);
	lv_obj_set_style_text_align(lbl_image_placeholder,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_size(lbl_image_placeholder,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_label_set_text(lbl_image_placeholder,_("No image found"));
	lv_obj_center(lbl_image_placeholder);
	lv_obj_set_hidden(lbl_image_placeholder,!images.empty());
	lv_obj_set_style_layout(lst_image,images.empty()?LV_LAYOUT_NONE:LV_LAYOUT_FLEX,0);

	auto last_id=installer_context["image"]["id"].asString();
	if(!last_id.empty())for(auto&img:images){
		if(img->id!=last_id)continue;
		image_set_selected(img,true);
		set_selected_image(img);
		lv_obj_scroll_to_view(img->btn_body,LV_ANIM_OFF);
		break;
	}
}
