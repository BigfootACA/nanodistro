#include"internal.h"
#include"error.h"
#include"log.h"
#include<list>

std::list<lv_image_decoder_t*>image_decoders{};

int image_init(){
	std::string backend;
	lv_image_decoder_t*decoder;
	for(auto&backend:image_backend_creates){
		auto img=backend.create();
		if(!img)throw RuntimeError("failed to create image decoder backend {}",backend.name);
		decoder=img->init();
		if(!decoder)throw RuntimeError("failed to initialize image decoder backend {}",backend.name);
		image_decoders.push_back(decoder);
	}
	return 0;
}
