#include"internal.h"

extern std::shared_ptr<input_backend>input_backend_create_sdl2_keyboard();
extern std::shared_ptr<input_backend>input_backend_create_sdl2_mouse();
extern std::shared_ptr<input_backend>input_backend_create_sdl2_mousewheel();
extern std::shared_ptr<input_backend>input_backend_create_evdev();
const std::vector<input_backend_create>input_backend_creates={
	#if defined(USE_LIB_SDL2)
	{"sdl2-keyboard",    input_backend_create_sdl2_keyboard},
	{"sdl2-mouse",       input_backend_create_sdl2_mouse},
	{"sdl2-mousewheel",  input_backend_create_sdl2_mousewheel},
	#endif
	{"evdev",            input_backend_create_evdev},
};
