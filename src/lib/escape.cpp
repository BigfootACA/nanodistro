#include<cstring>
#include<algorithm>
#include"str-utils.h"
#include"error.h"

int escape_len(char c){
	switch(c){
		case 'a':case 'b':case 'e':case 'f':
		case 'n':case 'r':case 't':case 'v':
		case '\\':case '\'':case '"':case '?':
			return 1;
		case '0':case '1':case '2':case '3':
		case '4':case '5':case '6':case '7':
			return 3;
		case 'x':return 3;
		case 'u':return 5;
		case 'U':return 9;
		default:return -1;
	}
}

std::string escape_parse(const std::string&esc){
	size_t idx=0;
	if(esc.empty())return "";
	switch(esc[0]){
		case 'a':return char_to_string(0x07);
		case 'b':return char_to_string(0x08);
		case 'e':return char_to_string(0x1B);
		case 'f':return char_to_string(0x0C);
		case 'n':return char_to_string(0x0A);
		case 'r':return char_to_string(0x0D);
		case 't':return char_to_string(0x09);
		case 'v':return char_to_string(0x0B);
		case '\\':return char_to_string(0x5C);
		case '\'':return char_to_string(0x27);
		case '"':return char_to_string(0x22);
		case '?':return char_to_string(0x3F);
		case '0':case '1':case '2':case '3':
		case '4':case '5':case '6':case '7':{
			auto len=std::min(esc.length(),(size_t)3);
			int d=std::stoi(esc.substr(0,len),&idx,8);
			if(idx<=0)throw InvalidArgument("bad escape");
			return char_to_string(d);
		}
		case 'x':{
			int c=std::stoi(esc.substr(1,2),&idx,16);
			if(idx!=2)throw InvalidArgument("bad escape");
			return char_to_string(c);
		}
		case 'u':{
			char32_t c=std::stoi(esc.substr(1,4),&idx,16);
			if(idx!=4)throw InvalidArgument("bad escape");
			return str_unicode_to_utf8(c);
		}
		case 'U':{
			char32_t c=std::stoul(esc.substr(1,8),&idx,16);
			if(idx!=8)throw InvalidArgument("bad escape");
			if(c>0x10FFFF)throw InvalidArgument("invalid unicode code point");
			return str_unicode_to_utf8(c);
		}
		default:throw InvalidArgument("unsupported escape");
	}
}
