#include"json-utils.h"
#include"str-utils.h"
#include"error.h"

void merge_json(Json::Value&dst,const Json::Value&src){
	if(src.isObject()&&dst.isObject()){
		if(dst.isMember("$delete"))
			for(const auto&key:dst["$delete"])
				dst.removeMember(key.asString());
		for(const auto&key:src.getMemberNames())
			merge_json(dst[key],src[key]);
	}else if(src.isArray()&&dst.isArray())
		for(const auto&i:src)
			dst.append(i);
	else dst=src;
}

void yaml_to_json(Json::Value&dst,const YAML::Node&src){
	if(!src.IsDefined())return;
	else if(src.IsNull())dst={Json::nullValue};
	else if(src.IsScalar()){
		auto val=src.as<std::string>();
		if(is_number(val))
			dst=src.as<int64_t>();
		else if(string_is_false(val)||string_is_true(val))
			dst=src.as<bool>();
		else dst=val;
	}else if(src.IsSequence()){
		dst={Json::arrayValue};
		for(const auto&i:src){
			Json::Value item{};
			yaml_to_json(item,i);
			dst.append(item);
		}
	}else if(src.IsMap()){
		dst={Json::objectValue};
		for(const auto&i:src)
			yaml_to_json(dst[i.first.as<std::string>()],i.second);
	}else throw RuntimeError("unsupported node");
}

void json_to_yaml(YAML::Node&dst,const Json::Value&src){
	if(src.isNull())dst=YAML::Null;
	else if(src.isBool())dst=src.asBool();
	else if(src.isUInt64())dst=src.asUInt64();
	else if(src.isInt64())dst=src.asInt64();
	else if(src.isUInt())dst=src.asUInt();
	else if(src.isInt())dst=src.asInt();
	else if(src.isDouble())dst=src.asDouble();
	else if(src.isString())dst=src.asString();
	else if(src.isArray())for(const auto&i:src){
		YAML::Node item{};
		json_to_yaml(item,i);
		dst.push_back(item);
	}else if(src.isObject())for(const auto&i:src.getMemberNames()){
		YAML::Node item{};
		json_to_yaml(item,src[i]);
		dst[i]=item;
	}else throw RuntimeError("unsupported node");
}

std::map<std::string,std::string>json_to_string_map(const Json::Value&src){
	std::map<std::string,std::string>ret{};
	if(!src.isObject())throw RuntimeError("invalid json object");
	for(const auto&key:src.getMemberNames())
		ret[key]=src[key].asString();
	return ret;
}

std::list<std::string>json_to_string_list(const Json::Value&src){
	std::list<std::string>ret{};
	if(!src.isArray())throw RuntimeError("invalid json array");
	for(const auto&item:src)
		ret.push_back(item.asString());
	return ret;
}

std::vector<std::string>json_to_string_vector(const Json::Value&src){
	std::vector<std::string>ret{};
	if(!src.isArray())throw RuntimeError("invalid json array");
	for(const auto&item:src)
		ret.push_back(item.asString());
	return ret;
}

static void LookupNodeWith(std::function<void(const std::string&path)>push,const std::string&path,char sep){
	size_t pos=0;
	char last_quote=0;
	bool in_esc=false;
	size_t esc_len=0;
	std::string key{},esc{};
	if(path.empty()){
		push("");
		return;
	}
	if(path[0]==sep)pos++;
	auto resolve_key=[&]{
		if(key.empty())throw RuntimeError("invalid path '{}'",path);
		push(key);
		key.clear();
	};
	for(;pos<path.size();pos++){
		if(in_esc){
			esc+=path[pos];
			if(esc_len==0){
				auto r=escape_len(path[pos]);
				if(r<=0)throw RuntimeError("invalid escape {:c} in '{}'",path[pos],path);
				esc_len=r;
			}
			if(esc.length()>=esc_len){
				key+=escape_parse(esc);
				in_esc=false,esc_len=0;
				if(esc.length()!=esc_len)
					key+=esc.substr(esc_len);
				esc.clear();
			}
		}else if(last_quote){
			if(path[pos]==last_quote)last_quote=0;
			else if(last_quote!='\''&&path[pos]=='\\'){
				in_esc=true,esc_len=0;
				esc.clear();
			}else key+=path[pos];
		}else if(path[pos]=='"'||path[pos]=='\'')
			last_quote=path[pos];
		else if(path[pos]=='\\'){
			in_esc=true,esc_len=0;
			esc.clear();
		}else if(path[pos]==sep)
			resolve_key();
		else key+=path[pos];
	}
	if(in_esc){
		if(esc_len>0)key+=escape_parse(esc);
		else throw RuntimeError("unexpected end escape in {}",path);
	}
	if(last_quote)throw RuntimeError("unexpected end quote in {}",path);
	if(!key.empty())resolve_key();
}

Json::Value lookup_node(const Json::Value&root,const std::string&path,char sep){
	Json::Value null{Json::nullValue},xroot{root};
	Json::Value&node=xroot;
	LookupNodeWith([&](const std::string&key){
		if(key.empty()){
			node=null;
			return;
		}
		if(node.isObject()){
			node=node.get(key,null);
		}else if(node.isArray()){
			if(is_number(key)){
				size_t idx=0;
				auto num=std::stoull(key,&idx);
				if(idx!=key.length())
					throw RuntimeError("invalid number {} in {}",key,path);
				node=node.get(num,null);
			}else node=null;
		}else node=null;
	},path,sep);
	return node;
}

YAML::Node lookup_node(const YAML::Node&root,const std::string&path,char sep){
	YAML::Node null{YAML::Null},xroot{root};
	YAML::Node&node=xroot;
	LookupNodeWith([&](const std::string&key){
		if(key.empty()){
			node=null;
			return;
		}
		if(node.IsMap()){
			auto v=node[key];
			node=v.IsDefined()?v:null;
		}else if(node.IsSequence()){
			if(is_number(key)){
				size_t idx=0;
				auto num=std::stoull(key,&idx);
				if(idx!=key.length())
					throw RuntimeError("invalid number {} in {}",key,path);
				auto v=node[num];
				node=v.IsDefined()?v:null;
			}else node=null;
		}else node=null;
	},path,sep);
	return node;
}
