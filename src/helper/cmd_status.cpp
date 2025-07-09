#include<getopt.h>
#include"status.h"
#include"internal.h"
#include"error.h"

static int usage(int e){
	fprintf(e==0?stdout:stderr,
		"Usage: helper status TEXT... [OPTIONS]\n"
		"Options:\n"
		"  -h, --help          Show this help message\n"
	);
	return e;
}

static const option lopts[]={
	{"help", no_argument, NULL, 'h'},
	{NULL,0,NULL,0}
};
static const char*sopts="h";

int helper_status_main(int argc,char**argv){
	int o;
	while((o=getopt_long(argc,argv,sopts,lopts,NULL))!=-1)switch(o){
		case 'h':return usage(0);
		default:throw InvalidArgument("unknown option {:c}",o);
	}
	if(optind>=argc)return usage(1);
	std::string data{};
	for(int i=optind;i<argc;i++){
		if(i>optind)data+=' ';
		data+=argv[i];
	}
	installer_set_status(data);
	return 0;
}
