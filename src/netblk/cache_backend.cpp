#include<sys/stat.h>
#include"internal.h"
#include"error.h"

class netblk_backend_readcache:public netblk_backend,public block_cache{
	public:
		netblk_backend_readcache(const std::shared_ptr<netblk_backend>&backend,size_t max_io_sector,size_t sector);
		~netblk_backend_readcache()override=default;
		inline std::string get_name()const override{return backend->get_name()+"-readcache";}
		inline mode_t access()const override{return backend->access();}
		inline size_t get_size()const override{return size;}
		size_t read(size_t offset,void*buf,size_t size)override;
		void prefetch(size_t offset,size_t size)override;
};

netblk_backend_readcache::netblk_backend_readcache(
	const std::shared_ptr<netblk_backend>&backend,
	size_t max_io_sector,size_t sector
):block_cache(backend,max_io_sector,sector){
	if(!backend->can_access(S_IROTH))
		throw InvalidArgument("backend not readable");
	this->name=get_name();
	init();
}

void netblk_backend_readcache::prefetch(size_t offset,size_t size){
	cache(offset,size);
}

size_t netblk_backend_readcache::read(size_t offset,void*buf,size_t size){
	return cached_read(offset,buf,size);
}

std::shared_ptr<netblk_backend>block_cache::create_backend(
	const std::shared_ptr<netblk_backend>&backend,
	size_t max_io_sector,size_t sector
){
	return std::make_shared<netblk_backend_readcache>(
		backend,max_io_sector,sector
	);
}

std::shared_ptr<netblk_backend>block_cache::create_backend(
	const std::shared_ptr<netblk_backend>&backend,
	const Json::Value&opts
){
	auto sector=0x1000; /* 4KB */
	auto max_io_size=0x4000000; /* 64MB */
	if(opts.isMember("cache")){
		auto cache=opts["cache"];
		if(cache.isMember("max-io-size"))
			max_io_size=cache["max-io-size"].asUInt();
		if(cache.isMember("sector"))
			sector=cache["sector"].asUInt();
	}
	auto max_io_sector=max_io_size/sector;
	return create_backend(backend,max_io_sector,sector);
}
