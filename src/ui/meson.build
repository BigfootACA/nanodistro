nanodistro_ui_srcs = files([
	'confirm.cpp',
	'hello.cpp',
	'init.cpp',
	'inputbox.cpp',
	'lib.cpp',
	'main.cpp',
	'msgbox.cpp',
	'pages.cpp',
	'termview.cpp',
])

subdir('progress')
subdir('choose_image')
subdir('choose_disk')
subdir('network')
subdir('mass_storage')

nanodistro_ui = static_library(
	'nanodistro-ui',
	nanodistro_ui_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
)
nanodistro_comps += nanodistro_ui
