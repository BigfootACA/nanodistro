nanodistro_netif_srcs = files([
	'wlan/base.cpp',
	'wlan/client.cpp',
	'wlan/network.cpp',
	'wlan/poll.cpp',
	'wlan/socket.cpp',
	'wlan/start.cpp',
	'intf.cpp',
	'ipv4.cpp',
	'ipv4_cidr.cpp',
	'ipv4_mask.cpp',
	'ipv4_route.cpp',
	'ipv6.cpp',
	'ipv6_cidr.cpp',
	'ipv6_route.cpp',
	'mac.cpp',
	'net.cpp',
	'netlink.cpp',
	'resolvconf.cpp',
])

nanodistro_netif = static_library(
	'nanodistro-netif',
	nanodistro_netif_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args,
)
nanodistro_comps += nanodistro_netif
