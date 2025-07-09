nanodistro_helper_srcs = files([
	'cmd_progress.cpp',
	'cmd_status.cpp',
	'cmd_write.cpp',
	'copy.cpp',
	'main.cpp',
])
nanodistro_helper_deps = [
	libmount,
]

nanodistro_helper = static_library(
	'nanodistro-helper',
	nanodistro_helper_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
	dependencies: nanodistro_helper_deps,
)
nanodistro_comps += nanodistro_helper
