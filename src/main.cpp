#include<vector>
#include<string>
#include"main.h"
#include"str-utils.h"
#include"path-utils.h"

std::vector<std::string>args{};

static const std::vector<std::pair<std::string,int(*)(int,char**)>>mains{
	{"netblk",      netblk_main},
	{"flasher",     flasher_main},
	{"helper",      helper_main},
	{"nanodistro",  nanodistro_main},
};

int main(int argc,char**argv){
	for(int i=0;i<argc;i++)
		args.push_back(argv[i]);
	auto xname=path_basename(args[0]);
	auto env=str_get_env("NANODISTRO_MAIN");
	if(!env.empty())for(auto&[name,func]:mains)
		if(argc>0&&name==env)return func(argc,argv);
	if(argc>1&&xname=="nanodistro")for(auto&[name,func]:mains)if(args[1]==name){
		args.erase(args.begin());
		return func(argc-1,argv+1);
	}
	if(argc>0)for(auto&[name,func]:mains)
		if(argc>0&&xname==name)return func(argc,argv);
	fprintf(stderr,"unknown main: %s\n",argv[0]);
	return 1;
}
