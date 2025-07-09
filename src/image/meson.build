nanodistro_image_srcs = files([
	'image.cpp',
	'backends.cpp',
	'nanosvg.cpp',
])
nanodistro_image_deps = []

nanodistro_image = static_library(
	'nanodistro-image',
	nanodistro_image_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
	dependencies: nanodistro_image_deps,
)
nanodistro_comps += nanodistro_image
