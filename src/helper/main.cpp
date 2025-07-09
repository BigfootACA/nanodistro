#include"main.h"
#include"log.h"
#include"internal.h"
#include"status.h"

static int helper_usage(int e){
	fprintf(e==0?stdout:stderr,
		"Usage: helper COMMAND [OPTIONS]\n"
		"Commands:\n"
		"    write ...      Write an image to disk\n"
		"    status ...     Set report status in GUI\n"
		"    progress ...   Set progress value in GUI\n"
		"    help           Show this help message\n"
	);
	return e;
}

static int post_main(int argc,char**argv){
	int r;
	if((r=installer_init())!=0)return r;
	if((r=log_init())!=0)return r;
	if((r=locale_init())!=0)return r;
	argc--,argv++;
	std::string prog(argv[0]);
	if(prog=="progress")return helper_progress_main(argc,argv);
	if(prog=="status")return helper_status_main(argc,argv);
	if(prog=="write")return helper_write_main(argc,argv);
	if(prog=="help"&&prog=="--help"&&prog=="-h")
		return helper_usage(0);
	return helper_usage(1);
}

int helper_main(int argc,char**argv){
	try{
		return post_main(argc,argv);
	}catch(const std::exception&exc){
		log_exception(exc,"fatal exception in main");
		return 1;
	}
}
