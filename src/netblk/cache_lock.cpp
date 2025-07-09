#include"internal.h"

void block_cache::invalidate(){
	invalidate_sector_(0,total_sectors);
}

block_cache::sector_t block_cache::get_cached_size(){
	return get_cached_sectors_()*sector;
}

void block_cache::reduce_half(){
	reduce_sector_(get_cached_sectors_()/2);
}

void block_cache::touch_sector(sector_t sector){
	std::lock_guard<std::mutex>lock(mutex);
	touch_sector_(sector);
}

void block_cache::reduce_sector(sector_t max_cached){
	std::lock_guard<std::mutex>lock(mutex);
	reduce_sector_(max_cached);
}

void block_cache::invalidate_sector(sector_t offset,sector_t size){
	std::lock_guard<std::mutex>lock(mutex);
	invalidate_sector_(offset,size);
}

void block_cache::cache(size_t offset,size_t size){
	std::lock_guard<std::mutex>lock(mutex);
	cache_(offset,size);
}

void block_cache::cache_sector(sector_t offset,sector_t size){
	std::lock_guard<std::mutex>lock(mutex);
	cache_sector_(offset,size);
}

void block_cache::force_cache_sector(sector_t offset,sector_t size){
	std::lock_guard<std::mutex>lock(mutex);
	force_cache_sector_(offset,size);
}

block_cache::sector_t block_cache::get_cached_sectors(){
	std::lock_guard<std::mutex>lock(mutex);
	return get_cached_sectors_();
}

size_t block_cache::cached_read(size_t offset,void*buf,size_t size){
	std::lock_guard<std::mutex>lock(mutex);
	return cached_read_(offset,buf,size);
}
