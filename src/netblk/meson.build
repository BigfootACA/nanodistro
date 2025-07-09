nanodistro_netblk_srcs = files([
	'cache_backend.cpp',
	'cache_lock.cpp',
	'cache.cpp',
	'curl.cpp',
	'fuse3.cpp',
	'main.cpp',
	'mem.cpp',
	'nbd.cpp',
	'netblk.cpp',
	'netblks.cpp',
	'psi.cpp',
	'ublk.cpp',
])

nanodistro_netblk = static_library(
	'nanodistro-netblk',
	nanodistro_netblk_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
	dependencies: [liburing, libcurl, libfuse3],
)
nanodistro_comps += nanodistro_netblk

compiler = meson.get_compiler('cpp')
cfg_data.set('HAVE_LINUX_NBD_H', compiler.has_header('linux/nbd.h', required: false))
cfg_data.set('HAVE_LINUX_UBLK_CMD_H', compiler.has_header('linux/ublk_cmd.h', required: false))
