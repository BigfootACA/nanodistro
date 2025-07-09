#include<sys/mman.h>
#include<sys/prctl.h>
#include"internal.h"
#include"error.h"
#include"std-utils.h"

void block_cache::force_cache_sector_(sector_t offset,sector_t count){
	if(!mem)throw RuntimeError("not initialized");
	if(offset+count>total_sectors)throw RuntimeError("cache out of range");
	while(offset>0&&(offset%max_io_sector)!=0&&!is_cached[offset-1])
		offset--,count++;
	while(count>0){
		auto once=std::min(count,max_io_sector);
		if(once<max_io_sector){
			sector_t end=std::min(offset+max_io_sector,total_sectors);
			auto it=std::find(is_cached.begin()+offset+once,is_cached.begin()+end,true);
			once=std::max(once,(it-is_cached.begin())-offset);
		}
		size_t off=offset*sector,len=once*sector;
		auto ret=backend->do_read(off,(uint8_t*)mem+off,len);
		if(ret<=0)throw RuntimeError("read failed");
		size_t proc=ret/sector,unaligned=ret%sector;
		if(proc==0&&offset==total_sectors-1&&unaligned==size%sector)proc=1;
		else if(proc<=0||unaligned!=0)throw RuntimeError("read unaligned");
		std::fill(is_cached.begin()+offset,is_cached.begin()+offset+proc,true);
		for(sector_t i=offset;i<offset+proc;i++)
			touch_sector_(i);
		count=proc>count?0:count-proc;
		offset+=proc;
	}
}

void block_cache::cache_sector_(sector_t offset,sector_t count){
	if(!mem)throw RuntimeError("not initialized");
	bool found_start=false;
	sector_t once_offset=0;
	sector_t once_count=0;
	if(offset+count>total_sectors)throw RuntimeError("cache out of range");
	for(sector_t i=offset;i<offset+count;i++)if(!is_cached[i]){
		if(found_start){
			once_count++;
		}else{
			once_offset=i;
			once_count=1;
			found_start=true;
		}
	}else if(found_start){
		force_cache_sector_(once_offset,once_count);
		found_start=false;
	}
	if(found_start)
		force_cache_sector_(once_offset,once_count);
}

static inline void size_to_sector(
	size_t in_offset,
	size_t in_size,
	block_cache::sector_t&out_offset,
	block_cache::sector_t&out_size,
	size_t sector
){
	if((in_offset%sector)!=0){
		auto aligned=align_down(in_offset,sector);
		in_size+=aligned-in_offset;
		in_offset=aligned;
	}
	if((in_size%sector)!=0)
		in_size=align_up(in_size,sector);
	out_offset=in_offset/sector;
	out_size=in_size/sector;
}

void block_cache::cache_(size_t offset,size_t size){
	if(!mem)throw RuntimeError("not initialized");
	if(size==0||offset>=this->size)return;
	if(offset+size>this->size)size=this->size-offset;
	sector_t sector_offset=0,sector_size=0;
	size_to_sector(offset,size,sector_offset,sector_size,sector);
	cache_sector_(sector_offset,sector_size);
}

size_t block_cache::cached_read_(size_t offset,void*buf,size_t size){
	if(!mem)throw RuntimeError("not initialized");
	if(offset>=this->size)return 0;
	auto real=std::min(size,this->size-offset);
	cache_(offset,real);
	memcpy(buf,(uint8_t*)mem+offset,real);
	return real;
}

void block_cache::reduce_sector_(sector_t max_cached){
	auto cached=get_cached_sectors_();
	while(cached>max_cached&&!lru_list.empty()){
		sector_t victim=lru_list.back();
		invalidate_sector_(victim,1);
		cached--;
	}
}

void block_cache::touch_sector_(sector_t sector){
	auto it=lru_map.find(sector);
	if(it!=lru_map.end())
		lru_list.erase(it->second);
	lru_list.push_front(sector);
	lru_map[sector]=lru_list.begin();
}

void block_cache::invalidate_sector_(sector_t offset,sector_t size){
	if(!mem)throw RuntimeError("not initialized");
	if(size==0||offset>=total_sectors)return;
	if(offset+size>total_sectors)size=total_sectors-offset;
	std::fill(is_cached.begin()+offset,is_cached.begin()+offset+size,false);
	madvise((uint8_t*)mem+offset*sector,size*sector,MADV_DONTNEED);
	for(sector_t i=offset;i<offset+size;i++){
		auto it=lru_map.find(i);
		if(it!=lru_map.end()){
			lru_list.erase(it->second);
			lru_map.erase(it);
		}
	}
}

block_cache::sector_t block_cache::get_cached_sectors_(){
	if(!mem)throw RuntimeError("not initialized");
	return std::count(is_cached.begin(),is_cached.end(),true);
}

void block_cache::init(){
	if(!backend)throw InvalidArgument("backend not set");
	if(this->size!=0||this->total_sectors!=0||this->mem)
		throw InvalidArgument("already initialized");
	if(this->sector==0||this->max_io_sector==0)
		throw InvalidArgument("sector size or max io sector not set");
	this->size=backend->get_size();
	this->total_sectors=align_up(size,sector)/sector;
	if(size<=0||total_sectors<=0)
		throw InvalidArgument("invalid backend size: {}",size);
	alloc_size=align_up(this->size,page);
	auto ptr=mmap(
		nullptr,alloc_size,
		PROT_READ|PROT_WRITE,
		MAP_PRIVATE|MAP_ANONYMOUS,-1,0
	);
	if(!ptr||ptr==MAP_FAILED)
		throw ErrnoError("mmap failed");
	prctl(
		PR_SET_VMA,PR_SET_VMA_ANON_NAME,
		(uintptr_t)ptr,alloc_size,name.c_str()
	);
	this->mem=ptr;
	this->is_cached.resize(total_sectors);
	invalidate_sector_(0,total_sectors);
	psi_monitor::push(this);
}

void block_cache::destroy()noexcept{
	if(mem){
		psi_monitor::pop(this);
		munmap(mem,alloc_size);
		mem=nullptr;
	}
	is_cached.clear();
	lru_list.clear();
	lru_map.clear();
	size=0;
	alloc_size=0;
	total_sectors=0;
}

block_cache::block_cache(
	const std::shared_ptr<netblk_backend>&backend,
	size_t max_io_sector,size_t sector
):backend(backend),sector(sector),max_io_sector(max_io_sector){
	if(!backend)throw InvalidArgument("backend not set");
	if(sector==0||max_io_sector==0)
		throw InvalidArgument("sector size or max io sector not set");
	page=sysconf(_SC_PAGESIZE);
	if(sector<page||sector%page!=0)
		throw InvalidArgument("sector size must be page aligned");
}

void block_cache::invalidate(size_t offset,size_t size){
	if(!mem)throw RuntimeError("not initialized");
	if(size==0||offset>=this->size)return;
	if(offset+size>this->size)size=this->size-offset;
	sector_t sector_offset=0,sector_size=0;
	size_to_sector(offset,size,sector_offset,sector_size,sector);
	invalidate_sector_(sector_offset,sector_size);
}
