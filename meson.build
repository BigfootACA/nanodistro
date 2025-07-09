project(
	'nanodistro',
	['c','cpp'],
	version: '0.1',
	default_options: [
		'warning_level=3',
		'cpp_std=c++20',
		'c_std=gnu23',
		'werror=true',
	]
)
nanodistro_inc = [
	include_directories('include')
]
nanodistro_comps = []
nanodistro_c_args = []
nanodistro_cpp_args = []
nanodistro_args = [
	'-Wno-unused-parameter',
	'-Wno-type-limits',
	'-Wno-sign-compare',
	'-Wno-missing-field-initializers',
	'-include', './config.h',
]

cfg_data = configuration_data()
cfg_data.set('VERSION', meson.project_version())

libfreetype2 = dependency('freetype2', required: true)
libpng = dependency('libpng', required: false)
libjpeg = dependency('libjpeg', required: false)
libsdl2 = dependency('sdl2', required: false)
libdrm = dependency('libdrm', required: false)
libcurl = dependency('libcurl', required: true)
libjsoncpp = dependency('jsoncpp', required: true)
libyamlcpp = dependency('yaml-cpp', required: true)
liburing = dependency('liburing', required: false)
libfuse3 = dependency('fuse3', required: false)
libkmod = dependency('libkmod', required: false)
libmount = dependency('mount', required: true)
libblkid = dependency('blkid', required: false)
libintl = dependency('intl', required: false)
cfg_data.set('USE_LIB_SDL2', libsdl2.found())
cfg_data.set('USE_LIB_LIBPNG', libpng.found())
cfg_data.set('USE_LIB_LIBJPEG', libjpeg.found())
cfg_data.set('USE_LIB_LIBDRM', libdrm.found())
cfg_data.set('USE_LIB_FUSE3', libfuse3.found())
cfg_data.set('USE_LIB_LIBURING', liburing.found())
cfg_data.set('USE_LIB_LIBKMOD', libkmod.found())
cfg_data.set('USE_LIB_LIBMOUNT', libmount.found())
cfg_data.set('USE_LIB_LIBBLKID', libblkid.found())

subdir('libs')
subdir('src')
subdir('po')

configure_file(
	input: 'src/config.h.in',
	output: 'config.h',
	configuration: cfg_data,
)

nanodistro_main_srcs = files([
	'src/main.cpp',
	'src/builtin.S',
])
executable(
	'nanodistro',
	nanodistro_main_srcs,
	install: true,
	include_directories: nanodistro_inc,
	link_with: nanodistro_comps,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
	dependencies: [libjsoncpp, libyamlcpp, libcurl, libkmod, libintl]
)
