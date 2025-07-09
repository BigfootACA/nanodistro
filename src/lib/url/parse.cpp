#include"url.h"
#include"error.h"

void url::parse(const char*u,size_t len){
	size_t s;
	char*c,*p=(char*)u;
	if(!u||!*u)return;
	if(len<=0)len=strlen(u);
	while(len>0&&isspace(u[0]))u++,len--;
	while(len>0&&isspace(u[len-1]))len--;
	if(len<=0)return;
	clear();
	if((c=strstr(p,"://"))){
		if((s=c-p)>0)set_scheme(p,s);
		p=c+3;
	}
	if((c=strpbrk(p,"@/"))&&*c=='@'){
		char*info=p;
		size_t si=c-p;
		p=c+1;
		if((c=(char*)memchr(info,':',si))){
			s=c-info;
			set_username(info,s);
			set_password(c+1,p-c-2);
		}else set_username(info,si);
	}
	if(*p=='['){
		if(!(c=strchr(p,']')))
			throw RuntimeError("missing ']'");
		set_host(p+1,c-p-1);
		p=c+1;
	}else if(*p!='/'&&*p!=':'&&*p!='?'&&*p!='#'){
		c=strpbrk(p,":/?#"),s=c?(size_t)(c-p):0;
		set_host(p,s);
		if(!c)return;
		p=c;
	}
	if(*p==':'){
		p++,c=strpbrk(p,"/?#"),s=c?(size_t)(c-p):0;
		set_port(p,s);
		if(!c)return;
		p=c;
	}
	if(*p=='/'){
		c=strpbrk(p,"?#");
		set_path(p,c?c-p:0);
		if(!c)return;
		p=c;
	}
	if(*p=='?'){
		p++,c=strchr(p,'#');
		set_query(p,c?c-p:0);
		if(!c)return;
		p=c;
	}
	if(*p=='#')set_fragment(p+1,0);
}
