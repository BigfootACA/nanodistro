#include<map>
#include<sys/stat.h>
#include<curl/curl.h>
#include"internal.h"
#include"readable.h"
#include"request.h"
#include"error.h"

class netblk_backend_curl;

class curl_wrapper{
	public:
		CURL*curl=nullptr;
		~curl_wrapper();
		curl_wrapper(CURL*curl);
};

struct curl_thread_ctx{
	curl_thread_ctx();
	~curl_thread_ctx();
	std::map<netblk_backend_curl*,std::shared_ptr<curl_wrapper>>m;
};

class netblk_backend_curl:
	public netblk_backend,
	public std::enable_shared_from_this<netblk_backend_curl>
{
	public:
		inline netblk_backend_curl()=default;
		~netblk_backend_curl();
		inline std::string get_name()const override{return "curl";}
		inline size_t get_size()const override{return size;}
		inline mode_t access()const override{return S_IROTH;}
		size_t read(size_t offset,void*buf,size_t size)override;
		std::string get_user_agent();
		void fill_opt(CURL*curl);
		void init();
		CURL*get_curl();
		std::mutex mutex{};
		std::string url{};
		size_t size=0;
		curl_slist*headers=nullptr;
		std::string ua{};
};

static thread_local bool curl_map_valid=false;
static thread_local curl_thread_ctx curl_map;

curl_wrapper::curl_wrapper(CURL*curl):curl(curl){}

curl_thread_ctx::curl_thread_ctx(){
	curl_map_valid=true;
}

curl_thread_ctx::~curl_thread_ctx(){
	curl_map_valid=false;
	m.clear();
}

curl_wrapper::~curl_wrapper(){
	if(curl)curl_easy_cleanup(curl);
	curl=nullptr;
}

std::shared_ptr<netblk_backend>netblk_backend_create_curl(const Json::Value&opts){
	auto backend=std::make_shared<netblk_backend_curl>();
	backend->url=opts["url"].asString();
	if(backend->url.empty())
		throw RuntimeError("curl backend need url");
	if(opts.isMember("headers"))for(auto&key:opts["headers"].getMemberNames()){
		auto header=std::format("{}: {}",key,opts["headers"][key].asString());
		backend->headers=curl_slist_append(backend->headers,header.c_str());
	}
	if(opts.isMember("user-agent"))
		backend->ua=opts["user-agent"].asString();
	backend->init();
	return backend;
}

std::shared_ptr<netblk_backend>netblk_backend_create_curl_readcache(const Json::Value&opts){
	auto backend=netblk_backend_create_curl(opts);
	return block_cache::create_backend(backend,opts);
}

void netblk_backend_curl::init(){
	CURLcode res;
	long code=0;
	curl_off_t size=0;
	auto curl=get_curl();
	curl_header*type=nullptr;
	fill_opt(curl);
	curl_easy_setopt(curl,CURLOPT_NOBODY,1L);
	if((res=curl_easy_perform(curl))!=CURLE_OK)
		throw RuntimeError("curl {} head failed: {}",url,curl_easy_strerror(res));
	if((res=curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&code))!=CURLE_OK)
		throw RuntimeError("curl {} get response code failed: {}",url,curl_easy_strerror(res));
	if(code!=200&&code!=204)throw RuntimeError("curl {} response not ok: {}",url,code);
	if(curl_easy_header(curl,"Accept-Ranges",0,CURLH_HEADER,-1,&type)!=CURLHE_OK)
		throw RuntimeError("curl {} get header accept-ranges failed",url);
	if(std::string(type->value)!="bytes")
		throw RuntimeError("{} accept-ranges not bytes",url);
	if((res=curl_easy_getinfo(curl,CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,&size))!=CURLE_OK)
		throw RuntimeError("curl {} get size failed: {}",url,curl_easy_strerror(res));
	if(size<0)throw RuntimeError("{} no content length",url);
	if(size<0x1000)throw RuntimeError("{} size too small",url);
	log_info("curl {} content length {}",url,size_string_float(size));
	this->size=size;
}

std::string netblk_backend_curl::get_user_agent(){
	if(!this->ua.empty())return this->ua;
	auto ua=request_user_agent();
	if(!ua.empty())ua+=' ';
	ua+="netblk";
	return ua;
}

void netblk_backend_curl::fill_opt(CURL*curl){
	curl_write_callback wcb=[](auto,auto a,auto b,auto){return a*b;};
	curl_easy_setopt(curl,CURLOPT_USERAGENT,get_user_agent().c_str());
	curl_easy_setopt(curl,CURLOPT_RANGE,"");
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,wcb);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,nullptr);
	curl_easy_setopt(curl,CURLOPT_NOBODY,0L);
	curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
	curl_easy_setopt(curl,CURLOPT_FAILONERROR,1L);
	curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);
	curl_easy_setopt(curl,CURLOPT_TIMEOUT,30L);
	curl_easy_setopt(curl,CURLOPT_TCP_KEEPALIVE,1L);
	curl_easy_setopt(curl,CURLOPT_TCP_KEEPIDLE,300L);
	curl_easy_setopt(curl,CURLOPT_TCP_KEEPINTVL,20L);
	curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
}

netblk_backend_curl::~netblk_backend_curl(){
	if(headers)curl_slist_free_all(headers);
	if(curl_map_valid)curl_map.m.erase(this);
}

CURL*netblk_backend_curl::get_curl(){
	if(curl_map.m.contains(this))
		return curl_map.m[this]->curl;
	auto n=curl_easy_init();
	if(!n)throw RuntimeError("curl init failed");
	curl_map.m[this]=std::make_shared<curl_wrapper>(n);
	return n;
}

struct write_ctx{
	char*buf=nullptr;
	size_t size=0;
	size_t left=0;
	size_t written=0;
};

static size_t write_cb(char*ptr,size_t sz,size_t nmemb,write_ctx*userdata){
	auto ctx=(write_ctx*)userdata;
	size_t to_copy=sz*nmemb;
	if(to_copy>ctx->left)to_copy=ctx->left;
	if(to_copy==0)return 0;
	if(ctx->written>=ctx->size)return 0;
	if(ctx->written+to_copy>ctx->size)
		to_copy=ctx->size-ctx->written;
	memcpy(ctx->buf+ctx->written,ptr,to_copy);
	ctx->written+=to_copy;
	ctx->left-=to_copy;
	return to_copy;
}

size_t netblk_backend_curl::read(size_t offset,void*buf,size_t size){
	long code=0;
	CURLcode res;
	auto curl=get_curl();
	if(!buf||size==0)return 0;
	if(offset+size>this->size)return 0;
	fill_opt(curl);
	auto range=std::format("{}-{}",offset,offset+size-1);
	curl_easy_setopt(curl,CURLOPT_RANGE,range.c_str());
	write_ctx ctx={(char*)buf,size,size,0};
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,(curl_write_callback)write_cb);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,&ctx);
	log_debug("curl read offset {} length {}",offset,size);
	if((res=curl_easy_perform(curl))!=CURLE_OK)
		throw RuntimeError("curl {} read failed: {}",url,curl_easy_strerror(res));
	if((res=curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&code))!=CURLE_OK)
		throw RuntimeError("curl {} get response code failed: {}",url,curl_easy_strerror(res));
	if(code==416||code==206)return ctx.written;
	throw RuntimeError("curl {} response not ok: {}",url,code);
}
