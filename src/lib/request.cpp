#include<curl/curl.h>
#include"str-utils.h"
#include"readable.h"
#include"cleanup.h"
#include"request.h"
#include"configs.h"
#include"error.h"
#include"log.h"

struct curl_write_ctx{
	std::string data;
	size_t size;
};

std::string request_user_agent(){
	std::string ua{};
	ua+="nanodistro";
	#ifdef NANODISTO_VERSION
	ua+="/" NANODISTO_VERSION;
	#endif
	ua+=" (libcurl";
	#ifdef LIBCURL_VERSION
	ua+="/" LIBCURL_VERSION;
	#endif
	ua+=")";
	return ua;
}

static size_t curl_write_func(char*ptr,size_t size,size_t nmemb,void*userdata){
	auto resp=(curl_write_ctx*)userdata;
	size_t realsize=size*nmemb;
	resp->data.append(ptr,realsize);
	resp->size+=realsize;
	return realsize;
}

static std::string perform(CURL*curl,curl_write_ctx*ctx){
	long code;
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,curl_write_func);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,ctx);
	curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);
	curl_easy_setopt(curl,CURLOPT_TIMEOUT,10L);
	curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,10L);
	curl_easy_setopt(curl,CURLOPT_FAILONERROR,1L);
	curl_easy_setopt(curl,CURLOPT_VERBOSE,0L);
	curl_easy_setopt(curl,CURLOPT_NOPROGRESS,1L);
	curl_easy_setopt(curl,CURLOPT_USERAGENT,request_user_agent().c_str());
	CURLcode res=curl_easy_perform(curl);
	if(res!=CURLE_OK)throw RuntimeError("curl error {}: {}",(int)res,curl_easy_strerror(res));
	curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&code);
	if(code<200||code>=300)throw RuntimeError("http error {}: {}",(int)code,ctx->data);
	log_info("http response code {} length {}",code,size_string_float(ctx->data.length()));
	return ctx->data;
}

std::string request_get_data(const url&u){
	curl_write_ctx ctx{};
	auto curl=curl_easy_init();
	if(!curl)throw RuntimeError("failed to create curl handle");
	cleanup_func curl_cleanup(std::bind(curl_easy_cleanup,curl));
	log_info("start request url {}",u.get_url());
	curl_easy_setopt(curl,CURLOPT_URL,u.get_url().c_str());
	curl_easy_setopt(curl,CURLOPT_HTTPGET,1L);
	curl_easy_setopt(curl,CURLOPT_NOBODY,0L);
	if(auto v=config["nanodistro"]["network"]["cacert"])
		curl_easy_setopt(curl,CURLOPT_CAINFO,v.as<std::string>().c_str());
	return perform(curl,&ctx);
}

Json::Value request_get_json(const url&u){
	Json::Value json;
	Json::Reader reader;
	curl_write_ctx ctx{};
	curl_slist*headers=nullptr;
	curl_slist_append(headers,"Accept: application/json");
	cleanup_func headers_cleanup(std::bind(curl_slist_free_all,headers));
	auto curl=curl_easy_init();
	if(!curl)throw RuntimeError("failed to create curl handle");
	cleanup_func curl_cleanup(std::bind(curl_easy_cleanup,curl));
	log_info("start request url {}",u.get_url());
	curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl,CURLOPT_URL,u.get_url().c_str());
	curl_easy_setopt(curl,CURLOPT_HTTPGET,1L);
	curl_easy_setopt(curl,CURLOPT_NOBODY,0L);
	if(auto v=config["nanodistro"]["network"]["cacert"])
		curl_easy_setopt(curl,CURLOPT_CAINFO,v.as<std::string>().c_str());
	auto data=perform(curl,&ctx);
	if(!reader.parse(data,json))
		throw RuntimeError("failed to parse json: {}",reader.getFormattedErrorMessages());
	return json;
}

Json::Value request_post_json(const url&u,const Json::Value&req){
	Json::Value json;
	Json::Reader reader;
	Json::FastWriter writer;
	curl_write_ctx ctx{};
	curl_slist*headers=nullptr;
	auto reqs=writer.write(req);
	auto curl=curl_easy_init();
	if(!curl)throw RuntimeError("failed to create curl handle");
	cleanup_func curl_cleanup(std::bind(curl_easy_cleanup,curl));
	curl_slist_append(headers,"Accept: application/json");
	curl_slist_append(headers,"Content-Type: application/json");
	curl_slist_append(headers,"Charset: utf-8");
	cleanup_func headers_cleanup(std::bind(curl_slist_free_all,headers));
	log_info("start request url {}",u.get_url());
	curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl,CURLOPT_URL,u.get_url().c_str());
	curl_easy_setopt(curl,CURLOPT_POST,1L);
	curl_easy_setopt(curl,CURLOPT_NOBODY,0L);
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,reqs.c_str());
	if(auto v=config["nanodistro"]["network"]["cacert"])
		curl_easy_setopt(curl,CURLOPT_CAINFO,v.as<std::string>().c_str());
	auto data=perform(curl,&ctx);
	if(!reader.parse(data,json))
		throw RuntimeError("failed to parse json: {}",reader.getFormattedErrorMessages());
	return json;
}

static size_t curl_header_func(
	const char*header,size_t size,size_t n,header_result*result
){
	std::string str(header,size*n);
	for(auto line:str_split(str,'\n')){
		if(line.empty())continue;
		auto idx=line.find(':');
		if(idx==std::string::npos)continue;
		auto key=str_trim_to(line.substr(0,idx));
		auto val=str_trim_to(line.substr(idx+1));
		for(auto&c:key)c=tolower(c);
		result->headers.insert({key,val});
	}
	return size*n;
}

std::shared_ptr<header_result>curl_header_helper(CURL*curl){
	auto result=std::make_shared<header_result>();
	curl_easy_setopt(curl,CURLOPT_HEADERDATA,result.get());
	curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,(curl_write_callback)curl_header_func);
	return result;
}
