nanodistro_input_srcs = files([
	'input.cpp',
	'backends.cpp',
])
nanodistro_input_deps = []

subdir('evdev')
subdir('sdl2')

nanodistro_input = static_library(
	'nanodistro-input',
	nanodistro_input_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
	dependencies: nanodistro_input_deps,
)
nanodistro_comps += nanodistro_input
