#include"url.h"
#include"error.h"

static char url_encoding_map[256];
static bool url_encoding_maps_initialized=false;

static void init_url_encoding_map(){
	unsigned char i;
	if(url_encoding_maps_initialized)return;
	memset(url_encoding_map,0,sizeof(url_encoding_map));
	for(i='a';i<='z';i++)url_encoding_map[i]=(char)i;
	for(i='A';i<='Z';i++)url_encoding_map[i]=(char)i;
	for(i='0';i<='9';i++)url_encoding_map[i]=(char)i;
	url_encoding_map[(uint32_t)'-']='-';
	url_encoding_map[(uint32_t)'_']='_';
	url_encoding_map[(uint32_t)'.']='.';
	url_encoding_map[(uint32_t)'*']='*';
	url_encoding_maps_initialized=true;
}

std::string url::encode(const std::string&src,const char*map,const std::string&skip){
	char m[256];
	if(src.empty())return "";
	if(!map){
		init_url_encoding_map();
		memcpy(m,url_encoding_map,sizeof(m));
	}else memcpy(m,map,sizeof(m));
	std::string out;
	for(char c:skip)m[(uint8_t)c]=c;
	for(char c:src){
		if(m[(uint8_t)c])out+=m[(uint8_t)c];
		else out+=std::format("%{0:2X}",c&0xFF);
	}
	return out;
}

std::string url::decode(const std::string&src){
	std::string out;
	size_t len=src.length();
	for(size_t i=0;i<len;i++)switch(src[i]){
		case '%':{
			size_t idx=0;
			if(len-i<3)throw RuntimeError("bad code");
			auto r=stoi(src.substr(i+1,2),&idx,16);
			if(idx!=2)throw RuntimeError("bad code hex");
			out+=(char)r;
		}break;
		case '+':out+=' ';break;
		default:out+=src[i];
	}
	return out;
}
