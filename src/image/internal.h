#ifndef DISPLAY_INTERNAL_H
#define DISPLAY_INTERNAL_H
#include<lvgl.h>
#include<memory>
#include<string>
#include<vector>

class image_backend{
	public:
		virtual lv_image_decoder_t*init()=0;
};

struct image_backend_create{
	const std::string name;
	std::shared_ptr<image_backend>(*create)();
};
extern const std::vector<image_backend_create>image_backend_creates;

#endif
