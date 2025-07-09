#ifndef JSON_UTILS_H
#define JSON_UTILS_H
#include<map>
#include<list>
#include<vector>
#include<json/json.h>
#include<yaml-cpp/yaml.h>
extern void merge_json(Json::Value&dst,const Json::Value&src);
extern void yaml_to_json(Json::Value&dst,const YAML::Node&src);
extern void json_to_yaml(YAML::Node&dst,const Json::Value&src);
extern std::map<std::string,std::string>json_to_string_map(const Json::Value&src);
extern std::list<std::string>json_to_string_list(const Json::Value&src);
extern std::vector<std::string>json_to_string_vector(const Json::Value&src);
extern Json::Value lookup_node(const Json::Value&root,const std::string&path,char sep='.');
extern YAML::Node lookup_node(const YAML::Node&root,const std::string&path,char sep='.');
#endif
