nanodistro_fonts_srcs = files([
	'fonts.cpp',
	'backends.cpp',
])
nanodistro_fonts_deps = []

subdir('freetype2')

nanodistro_fonts = static_library(
	'nanodistro-fonts',
	nanodistro_fonts_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
	dependencies: nanodistro_fonts_deps,
)
nanodistro_comps += nanodistro_fonts
