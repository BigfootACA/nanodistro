#include<getopt.h>
#include"status.h"
#include"internal.h"
#include"str-utils.h"
#include"error.h"

static int usage(int e){
	fprintf(e==0?stdout:stderr,
		"Usage: helper progress -p VALUE [OPTIONS]\n"
		"Options:\n"
		"  -p, --progress <VALUE>  Set progress value (Range 0-100)\n"
		"  -e, --enable <ENABLE>   Enable progress\n"
		"  -h, --help              Show this help message\n"
	);
	return e;
}

static const option lopts[]={
	{"help",     no_argument,       NULL, 'h'},
	{"progress", required_argument, NULL, 'p'},
	{"enable",   required_argument, NULL, 'e'},
	{NULL,0,NULL,0}
};
static const char*sopts="p:e:h";

int helper_progress_main(int argc,char**argv){
	int o;
	bool enable=false;
	int value=-1;
	while((o=getopt_long(argc,argv,sopts,lopts,NULL))!=-1)switch(o){
		case 'h':return usage(0);
		case 'p':{
			size_t idx=0;
			value=std::stoi(optarg,&idx);
			if(idx!=strlen(optarg)||value<0||value>100)
				throw InvalidArgument("invalid progress value");
		}break;
		case 'e':{
			if(string_is_true(optarg))enable=true;
			else if(string_is_false(optarg))enable=false;
			else throw InvalidArgument("invalid progress enable");
		}break;
		default:throw InvalidArgument("unknown option {:c}",o);
	}
	if(optind!=argc)return usage(1);
	if(value<0)return usage(1);
	installer_set_progress_enable(enable);
	installer_set_progress_value(value);
	return 0;
}
