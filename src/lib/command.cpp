#include<cctype>
#include<vector>
#include<functional>
#include"str-utils.h"
#include"error.h"

std::vector<std::string>parse_command(const std::string&cmd,std::function<std::string(const std::string&)>getvar){
	std::string buff{},sub{},esc{};
	std::vector<std::string>args{};
	size_t pos=0,esc_len=0;
	char last_quote=0;
	bool in_sub=false,in_esc=false,sub_end=false;
	auto finish_buff=[&]{
		if(buff.empty())return;
		args.push_back(buff);
		buff.clear();
	};
	auto finish_sub=[&]{
		if(sub.empty())throw RuntimeError("empty subscript");
		buff+=getvar(sub);
		in_sub=false,sub_end=false;
	};
	for(char c:cmd){
		pos++;
		if(in_esc){
			esc+=c;
			if(esc_len==0){
				auto r=escape_len(c);
				if(r<=0)throw RuntimeError("invalid escape {:c} in '{}'",c,cmd);
				esc_len=r;
			}
			if(esc.length()>=esc_len){
				buff+=escape_parse(esc);
				in_esc=false,esc_len=0;
				if(esc.length()!=esc_len)
					buff+=esc.substr(esc_len);
				esc.clear();
			}
			continue;
		}else if(in_sub){
			auto first=sub.empty();
			if(c=='{'&&first&&!sub_end){
				sub_end=true;
			}else if(c=='}'&&sub_end){
				finish_sub();
			}else if(!check_ident_char(c,first)){
				if(sub_end)throw RuntimeError(
					"bad char {:c} at pos {} in '{}'",
					c,pos,cmd
				);
				finish_sub();
			}else sub+=c;
			continue;
		}else if(std::isspace(c)&&!last_quote){
			finish_buff();
		}else if(c=='\\'&&last_quote!='\''){
			in_esc=true,esc_len=0;
			esc.clear();
		}else if(c=='$'&&last_quote!='\''){
			in_sub=true;
			sub.clear();
		}else if((c=='\''||c=='"')&&!last_quote){
			last_quote=c;
		}else buff+=c;
	}
	if(in_esc){
		if(esc_len>0)buff+=escape_parse(esc);
		else throw RuntimeError("unexpected end escape in {}",cmd);
	}
	if(last_quote)throw RuntimeError("unexpected end quote in {}",cmd);
	if(in_sub){
		if(sub_end)throw RuntimeError("unexpected end subscript in {}",cmd);
		finish_sub();
	}
	finish_buff();
	return args;
}
