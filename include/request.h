#ifndef REQUEST_H
#define REQUEST_H
#include"url.h"
#include<memory>
#include<json/json.h>
#include<curl/curl.h>
extern std::string request_get_data(const url&u);
extern Json::Value request_get_json(const url&u);
extern Json::Value request_post_json(const url&u,const Json::Value&req);
extern std::string request_user_agent();
struct header_result{
	std::multimap<std::string,std::string>headers{};
};
extern std::shared_ptr<header_result>curl_header_helper(CURL*curl);
#endif
