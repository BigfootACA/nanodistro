#include<thread>
#include<fcntl.h>
#include<getopt.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<sys/prctl.h>
#include<linux/fs.h>
#include"status.h"
#include"configs.h"
#include"readable.h"
#include"internal.h"
#include"time-utils.h"
#include"str-utils.h"
#include"fs-utils.h"
#include"error.h"

struct helper_write_options{
	int progress_start=0;
	int progress_end=100;
	bool report_status=true;
	bool report_progress=true;
	std::string input_file{};
	std::string output_device="auto";
	int partition=0;
	size_t block_size=4096;
	size_t input_offset=0;
	size_t output_offset=0;
	size_t count=0;
	size_t length=0;
	int report_period=1;
	int print_period=5;
	bool force_sync=false;
	bool open_sync=false;
};

class helper_write_context{
	public:
		~helper_write_context();
		void open_device();
		void valid_options();
		void report_main();
		void start_report();
		void stop_report();
		helper_write_options opts{};
		copy_context copy_ctx{};
		int in_fd=-1,out_fd=-1;
		std::thread thread{};
		std::atomic<bool>running=false;
};

static int usage(int e){
	fprintf(e==0?stdout:stderr,
		"Usage: helper write [OPTIONS]\n"
		"Options:\n"
		"  -S, --start <VALUE>      Start progress value (Range: 0-100, Default 0)\n"
		"  -E, --end <VALUE>        End progress value (Range: 0-100, Default 100)\n"
		"  -r, --report <ENABLE>    Enable or disable status report (Default enabled)\n"
		"  -p, --progress <ENABLE>  Enable or disable progress report (Default enabled)\n"
		"  -f, --file <FILE>        Source file\n"
		"  -d, --device <DEVICE>    Destination device (Default: read from config)\n"
		"  -P, --partition <PART>   Partition to write (Default: 0 for full disk)\n"
		"  -b, --blocksize <BLOCK>  Block size per IO (Default: 4 KiB)\n"
		"  -o, --offset <OFFSET>    Input file offset (Default: 0)\n"
		"  -s, --seek <SEEK>        Output device offset (Default: 0)\n"
		"  -c, --count <COUNT>      Number of blocks to write (Default: calc from source file size)\n"
		"  -l, --length <LENGTH>    Bytes to write (Default: source file size)\n"
		"  -i, --period <PERIOD>    Report period in seconds (Default: 1)\n"
		"  -t, --print <TIME>       Print period in seconds (Default: 5)\n"
		"  -F, --flush              Force sync after each write\n"
		"  -C, --sync               Open destination device with O_SYNC\n"
		"  -h, --help               Show this help message\n"
	);
	return e;
}

static const option lopts[]={
	{"help",     no_argument,       NULL, 'h'},
	{"start",    required_argument, NULL, 'S'},
	{"end",      required_argument, NULL, 'E'},
	{"report",   required_argument, NULL, 'r'},
	{"progress", required_argument, NULL, 'p'},
	{"file",     required_argument, NULL, 'f'},
	{"device",   required_argument, NULL, 'd'},
	{"blocksize",required_argument, NULL, 'b'},
	{"offset",   required_argument, NULL, 'o'},
	{"seek",     required_argument, NULL, 's'},
	{"count",    required_argument, NULL, 'c'},
	{"length",   required_argument, NULL, 'l'},
	{"period",   required_argument, NULL, 'i'},
	{"print",    required_argument, NULL, 't'},
	{"flush",    no_argument,       NULL, 'F'},
	{"sync",     no_argument,       NULL, 'C'},
	{NULL,0,NULL,0}
};
static const char*sopts="hS:E:r:p:f:d:b:o:s:c:l:i:t:FC";

static void load_int(const std::string&name,int&dst,int min=INT_MIN,int max=INT_MAX){
	size_t idx=0;
	std::string val=optarg;
	str_trim(val);
	auto v=std::stoi(val,&idx);
	if(idx!=val.length()||v<min||v>max)
		throw InvalidArgument("invalid {} value {}",name,val);
	dst=v;
}

static void load_size(const std::string&name,size_t&dst){
	static const std::string units="BKMGTPEZY";
	size_t idx=0;
	std::string val=optarg;
	str_trim(val);
	const auto len=val.length();
	auto v=std::stoull(val,&idx);
	if(idx==0)throw InvalidArgument("invalid {} value {}",name,val);
	if(idx!=len){
		while(idx<len&&std::isspace(val[idx]))idx++;
		if(idx!=len-1)throw InvalidArgument("invalid {} value {}",name,val);
		auto spec=std::toupper(val[idx]);
		auto pos=units.find(spec);
		if(pos==std::string::npos)
			throw InvalidArgument("invalid {} value {}",name,val);
		v*=1ULL<<(pos*10);
	}
	dst=v;
}

static void load_bool(const std::string&name,bool&dst){
	if(string_is_true(optarg))dst=true;
	else if(string_is_false(optarg))dst=false;
	else throw InvalidArgument("invalid {} value {}",name,optarg);
}

void helper_write_context::valid_options(){
	if(opts.input_file.empty())
		throw InvalidArgument("input file not set");
	if(opts.progress_start>opts.progress_end)
		throw InvalidArgument("progress start larger than end");
	if(opts.block_size==0)
		throw InvalidArgument("block size cannot be 0");
	if(opts.count!=0&&opts.length!=0&&opts.count*opts.block_size!=opts.length)
		throw InvalidArgument("count and length mismatch");
	if(opts.partition!=0&&opts.output_device!="auto")
		throw InvalidArgument("partition and device cannot be set at the same time");
}

void helper_write_context::open_device(){
	if(in_fd<0){
		in_fd=open(opts.input_file.c_str(),O_RDONLY|O_CLOEXEC);
		if(in_fd<0)throw ErrnoError("open input file failed",opts.input_file);
	}
	if(out_fd<0){
		if(opts.output_device=="auto"){
			installer_load_context();
			auto dev=installer_context["disk"]["device"].asString();
			if(dev.empty())throw InvalidArgument("output device not set");
			if(opts.partition!=0){
				if(std::isdigit(dev.back()))dev+='p';
				dev+=std::to_string(opts.partition);
			}
			log_info("use {} as output device",dev);
			opts.output_device=dev;
		}
		int flags=O_WRONLY|O_CLOEXEC;
		if(opts.open_sync)flags|=O_SYNC;
		out_fd=open(opts.output_device.c_str(),flags);
		if(out_fd<0)throw ErrnoError("open output device {} failed",opts.output_device);
	}
	if(opts.count==0&&opts.length==0){
		struct stat st;
		if(fstat(in_fd,&st)<0)throw ErrnoError("fstat input file {} failed",opts.input_file);
		size_t realsize=0;
		if(S_ISREG(st.st_mode))
			realsize=st.st_size;
		else if(S_ISBLK(st.st_mode))
			xioctl(in_fd,BLKGETSIZE64,&realsize);
		else throw InvalidArgument("input file {} is not a regular file or block device",opts.input_file);
		if(opts.input_offset>realsize)
			throw InvalidArgument("input offset {} larger than input file size {}",opts.input_offset,realsize);
		opts.length=realsize-opts.input_offset;
		if(opts.length==0)throw InvalidArgument("input file {} is empty",opts.input_file);
	}
	if(opts.count==0&&opts.length!=0)opts.count=(opts.length/opts.block_size)+(opts.length%opts.block_size)!=0;
	if(opts.count!=0&&opts.length==0)opts.length=opts.count*opts.block_size;
	struct stat st{};
	if(fstat(out_fd,&st)<0)throw ErrnoError("fstat output file {} failed",opts.input_file);
	size_t devsize=0;
	if(S_ISREG(st.st_mode))
		devsize=st.st_size;
	if(S_ISBLK(st.st_mode))
		xioctl(out_fd,BLKGETSIZE64,&devsize);
	if(devsize!=0&&opts.output_offset>devsize)
		throw InvalidArgument("output offset {} larger than output device size {}",opts.output_offset,devsize);
	devsize-=opts.output_offset;
	if(opts.length>devsize)
		throw InvalidArgument("length {} larger than output device size {}",opts.length,devsize);
}

void helper_write_context::report_main(){
	timestamp last_time{};
	timestamp last_report{};
	timestamp last_print{};
	const timestamp report_period(opts.report_period,timestamp::second);
	const timestamp print_period(opts.print_period,timestamp::second);
	size_t last_written=0;
	running=true;
	prctl(PR_SET_NAME,"write-report");
	while(running){
		sleep(1);
		auto now=timestamp::now();
		last_time=now;
		std::lock_guard<std::mutex>lock(copy_ctx.stat.lock);
		copy_ctx.stat.calc();
		if(copy_ctx.stat.cur.written==last_written)continue;
		last_written=copy_ctx.stat.cur.written;
		auto status=std::format(
			"{}/{} {}",
			format_size_float(copy_ctx.stat.cur.written),
			format_size_float(copy_ctx.stat.total),
			format_size_float(copy_ctx.stat.progress.speed,size_units_ibs)
		);
		if(now-last_report>=report_period){
			last_report=now;
			if(opts.report_status)installer_set_status(ssprintf(
				_("Writing %s: %s"),
				opts.input_file.c_str(),status.c_str()
			));
			if(opts.report_progress){
				int real_percent=std::clamp<int>(
					opts.progress_start+(opts.progress_end-opts.progress_start)*
					copy_ctx.stat.progress.percent/100,0,100
				);
				installer_set_progress_value(real_percent);
			}
		}
		if(now-last_print>=print_period){
			last_print=now;
			log_info(
				"write {} to {}: {} ({}%)",
				opts.input_file,opts.output_device,
				status,copy_ctx.stat.progress.percent
			);
		}
	}
}

void helper_write_context::start_report(){
	auto f=std::bind(&helper_write_context::report_main,this);
	thread=std::thread(f);
}

void helper_write_context::stop_report(){
	running=false;
	if(thread.joinable())thread.join();
}

helper_write_context::~helper_write_context(){
	stop_report();
	if(in_fd>=0)close(in_fd);
	if(out_fd>=0)close(out_fd);
	in_fd=-1,out_fd=-1;
}

int helper_write_main(int argc,char**argv){
	int o;
	helper_write_context ctx;
	while((o=getopt_long(argc,argv,sopts,lopts,NULL))!=-1)switch(o){
		case 'h':return usage(0);
		case 'S':load_int("progress start",ctx.opts.progress_start,0,100);break;
		case 'E':load_int("progress end",ctx.opts.progress_end,0,100);break;
		case 'r':load_bool("report status",ctx.opts.report_status);break;
		case 'p':load_bool("report progress",ctx.opts.report_progress);break;
		case 'f':ctx.opts.input_file=optarg;break;
		case 'd':ctx.opts.output_device=optarg;break;
		case 'P':load_int("partition",ctx.opts.partition,0);break;
		case 'b':load_size("block size",ctx.opts.block_size);break;
		case 'o':load_size("input offset",ctx.opts.input_offset);break;
		case 's':load_size("output seek",ctx.opts.output_offset);break;
		case 'c':load_size("count",ctx.opts.count);break;
		case 'l':load_size("length",ctx.opts.length);break;
		case 'i':load_int("report period",ctx.opts.report_period);break;
		case 't':load_int("print period",ctx.opts.print_period);break;
		case 'F':ctx.opts.force_sync=true;break;
		case 'C':ctx.opts.open_sync=true;break;
		default:throw InvalidArgument("unknown option {:c}",o);
	}
	if(optind!=argc)return usage(1);
	ctx.valid_options();
	if(ctx.opts.report_status)installer_set_status(ssprintf(
		_("Start write %s"),ctx.opts.input_file.c_str()
	));
	if(ctx.opts.report_progress){
		installer_set_progress_enable(true);
		installer_set_progress_value(ctx.opts.progress_start);
	}
	auto start=timestamp::now();
	ctx.open_device();
	ctx.copy_ctx.block_size=ctx.opts.block_size;
	ctx.copy_ctx.input_offset=ctx.opts.input_offset;
	ctx.copy_ctx.output_offset=ctx.opts.output_offset;
	ctx.copy_ctx.in_fd=ctx.in_fd;
	ctx.copy_ctx.out_fd=ctx.out_fd;
	ctx.copy_ctx.stat.total=ctx.opts.length;
	ctx.copy_ctx.sync=ctx.opts.force_sync;
	ctx.copy_ctx.stat.cur.time=start;
	ctx.start_report();
	ctx.copy_ctx.do_copy();
	ctx.stop_report();
	if(ctx.opts.report_status)installer_set_status(ssprintf(
		_("Syncing %s"),ctx.opts.output_device.c_str()
	));
	fsync(ctx.out_fd);
	sync();
	if(ctx.copy_ctx.stat.cur.written!=ctx.copy_ctx.stat.total)
		throw RuntimeError("write {} failed",ctx.opts.input_file);
	auto end=timestamp::now();
	auto elapsed=std::max<int64_t>(1,(end-start).tosecond());
	auto speed=ctx.copy_ctx.stat.cur.written/elapsed;
	log_info(
		"wrote {} speed {}",
		size_string_float(ctx.copy_ctx.stat.cur.written),
		format_size_float(speed,size_units_ibs)
	);
	if(ctx.opts.report_status)installer_set_status(ssprintf(
		_("Write %s done"),ctx.opts.input_file.c_str()
	));
	return 0;
}
