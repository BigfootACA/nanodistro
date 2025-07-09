nanodistro_display_srcs = files([
	'backends.cpp',
	'console.cpp',
	'display.cpp',
	'drm.cpp',
	'fbdev.cpp',
	'sdl2.cpp',
])
nanodistro_display_deps = [libdrm, libsdl2]

nanodistro_display = static_library(
	'nanodistro-display',
	nanodistro_display_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
	dependencies: nanodistro_display_deps,
)
nanodistro_comps += nanodistro_display
