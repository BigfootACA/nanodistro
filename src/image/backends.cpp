#include"internal.h"

extern std::shared_ptr<image_backend>image_backend_create_nanosvg();
const std::vector<image_backend_create>image_backend_creates={
	{"nanosvg",   image_backend_create_nanosvg},
};
