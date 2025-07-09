#include"internal.h"

extern std::shared_ptr<display_backend>display_backend_create_drm();
extern std::shared_ptr<display_backend>display_backend_create_fbdev();
extern std::shared_ptr<display_backend>display_backend_create_sdl2();
const std::vector<display_backend_create>display_backend_creates={
	#if defined(USE_LIB_LIBDRM)
	{"drm",       display_backend_create_drm},
	#endif
	{"fbdev",     display_backend_create_fbdev},
	#if defined(USE_LIB_SDL2)
	{"sdl2",      display_backend_create_sdl2},
	#endif
};
