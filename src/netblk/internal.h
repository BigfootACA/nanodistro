#ifndef NETBLK_INTERNAL_H
#define NETBLK_INTERNAL_H
#include<list>
#include<atomic>
#include<string>
#include<memory>
#include<mutex>
#include<thread>
#include<regex>
#include<functional>
#include<json/json.h>
class block_cache;
class netblk_backend;
class psi_monitor{
	public:
		psi_monitor();
		~psi_monitor();
		void loop();
		void check();
		void on_low();
		void add(block_cache*cache);
		void remove(block_cache*cache);
		static std::shared_ptr<psi_monitor>get();
		static void push(block_cache*cache);
		static void pop(block_cache*cache);
		std::atomic<bool>want_gc=false;
	private:
		static std::shared_ptr<psi_monitor>instance;
		std::thread thread;
		std::mutex mutex{};
		std::list<block_cache*>caches{};
		std::atomic<bool>running=true;
		std::regex re_avg10{R"(some avg10=(\d+\.\d+))"};
};
class block_cache{
	public:
		using sector_t=uint64_t;
		block_cache(const std::shared_ptr<netblk_backend>&backend,size_t max_io_sector,size_t sector=4096);
		inline virtual ~block_cache(){destroy();}
		virtual void init();
		virtual void destroy()noexcept;
		virtual void reduce_half();
		virtual void touch_sector(sector_t sector);
		virtual void reduce_sector(sector_t max_cached);
		virtual void invalidate();
		virtual void invalidate(size_t offset,size_t size);
		virtual void invalidate_sector(sector_t offset,sector_t size);
		virtual void cache(size_t offset,size_t size);
		virtual void cache_sector(sector_t offset,sector_t size);
		virtual void force_cache_sector(sector_t offset,sector_t size);
		virtual sector_t get_cached_sectors();
		virtual sector_t get_cached_size();
		virtual size_t cached_read(size_t offset,void*buf,size_t size);
		static std::shared_ptr<netblk_backend>create_backend(
			const std::shared_ptr<netblk_backend>&backend,
			size_t max_io_sector,size_t sector=4096
		);
		static std::shared_ptr<netblk_backend>create_backend(
			const std::shared_ptr<netblk_backend>&backend,
			const Json::Value&opts
		);
	protected:
		virtual void touch_sector_(sector_t sector);
		virtual void reduce_sector_(sector_t max_cached);
		virtual void invalidate_sector_(sector_t offset,sector_t size);
		virtual void cache_(size_t offset,size_t size);
		virtual void cache_sector_(sector_t offset,sector_t size);
		virtual void force_cache_sector_(sector_t offset,sector_t size);
		virtual sector_t get_cached_sectors_();
		virtual size_t cached_read_(size_t offset,void*buf,size_t size);
		std::string name="netblk-readcache";
		std::list<sector_t>lru_list{};
		std::unordered_map<sector_t,std::list<sector_t>::iterator>lru_map;
		std::shared_ptr<netblk_backend>backend=nullptr;
		std::vector<bool>is_cached{};
		std::mutex mutex{};
		void*mem=nullptr;
		size_t size=0,alloc_size=0;
		size_t sector=0,page=0;
		size_t total_sectors=0;
		sector_t max_io_sector=0;
};
class netblk_backend{
	public:
		virtual ~netblk_backend()=default;
		virtual std::string get_name()const=0;
		virtual size_t get_size()const=0;
		virtual mode_t access()const=0;
		virtual void prefetch(size_t offset,size_t size){}
		virtual size_t read(size_t offset,void*buf,size_t size){return 0;}
		virtual size_t write(size_t offset,const void*buf,size_t size){return 0;}
		bool can_access(mode_t mode);
		ssize_t do_read(size_t offset,void*buf,size_t size);
		ssize_t do_write(size_t offset,const void*buf,size_t size);
		static std::shared_ptr<netblk_backend>create(
			const std::string&type,
			const Json::Value&opts
		);
		int retry=3;
};
class netblk_device{
	public:
		virtual ~netblk_device()=default;
		virtual std::string get_path()const=0;
		virtual void destroy()=0;
		virtual std::shared_ptr<netblk_backend>get_backend()=0;
		virtual void setup_block(){}
};
class netblk_implement{
	public:
		virtual ~netblk_implement()=default;
		virtual std::string get_name()const=0;
		virtual void try_enable(){}
		virtual bool is_supported()const{return true;}
		virtual bool is_prefer()const{return true;}
		virtual bool check_supported();
		virtual void init()=0;
		virtual void stop(){}
		virtual void run();
		virtual void loop();
		virtual std::shared_ptr<netblk_device>create(const std::shared_ptr<netblk_backend>&backend)=0;
		static std::shared_ptr<netblk_implement>choose(const Json::Value&opts);
		std::atomic<bool>running=false;
};
using netblk_backend_create_func=std::function<std::shared_ptr<netblk_backend>(const Json::Value&opts)>;
using netblk_implement_create_func=std::function<std::shared_ptr<netblk_implement>(const Json::Value&opts)>;
extern const std::vector<std::pair<std::string,netblk_backend_create_func>>netblk_backend_list;
extern const std::vector<std::pair<std::string,netblk_implement_create_func>>netblk_implement_list;
#endif
