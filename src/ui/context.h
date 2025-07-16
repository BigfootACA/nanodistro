#ifndef CONTEXT_H
#define CONTEXT_H
#include<string>
#include<functional>
#include<json/json.h>
struct disk_context{
	std::function<void(const Json::Value&disk)>callback;
	std::string back_page{};
	std::string next_page{};
	std::string title{};
};
#endif
