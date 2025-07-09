#include"internal.h"

extern std::shared_ptr<netblk_backend>netblk_backend_create_mem(const Json::Value&opts);
extern std::shared_ptr<netblk_backend>netblk_backend_create_curl(const Json::Value&opts);
extern std::shared_ptr<netblk_backend>netblk_backend_create_curl_readcache(const Json::Value&opts);
extern std::shared_ptr<netblk_implement>netblk_implement_create_ublk(const Json::Value&opts);
extern std::shared_ptr<netblk_implement>netblk_implement_create_nbd(const Json::Value&opts);
extern std::shared_ptr<netblk_implement>netblk_implement_create_fuse3(const Json::Value&opts);

const std::vector<std::pair<std::string,netblk_backend_create_func>>netblk_backend_list{
	{"mem",            netblk_backend_create_mem},
	{"curl",           netblk_backend_create_curl},
	{"curl-readcache", netblk_backend_create_curl_readcache},
};

const std::vector<std::pair<std::string,netblk_implement_create_func>>netblk_implement_list{
	#if defined(USE_LIB_LIBURING)&&defined(HAVE_LINUX_UBLK_CMD_H)
	{"ublk",    netblk_implement_create_ublk},
	#endif
	#if defined(HAVE_LINUX_NBD_H)
	{"nbd",     netblk_implement_create_nbd},
	#endif
	#if defined(USE_LIB_FUSE3)
	{"fuse3",   netblk_implement_create_fuse3},
	#endif
};
