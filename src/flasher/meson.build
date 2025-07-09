nanodistro_flasher_srcs = files([
	'cleanup.cpp',
	'install.cpp',
	'lock.cpp',
	'main.cpp',
	'netblk.cpp',
])
nanodistro_flasher_deps = [
	libmount,
]

nanodistro_flasher = static_library(
	'nanodistro-flasher',
	nanodistro_flasher_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
	dependencies: nanodistro_flasher_deps,
)
nanodistro_comps += nanodistro_flasher
