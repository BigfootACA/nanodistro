#include"internal.h"

extern std::shared_ptr<font_backend>font_backend_create_freetype2();
const std::vector<font_backend_create>font_backend_creates={
	{"freetype2",    font_backend_create_freetype2},
};
