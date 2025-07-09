#ifndef INTERNAL_H
#define INTERNAL_H
#include<mutex>
#include<csignal>
#include"time-utils.h"
struct copy_status{
	size_t total=0;
	struct{
		size_t written=0;
		timestamp time=0;
	}elast{},clast{},cur{};
	struct{
		int percent=0;
		size_t written=0;
		size_t speed=0;
		timestamp passed=0;
	}progress;
	std::mutex lock{};
	void calc();
};

enum copy_ret{
	COPY_CONTINUE,
	COPY_FALLBACK,
	COPY_END,
};

class copy_context{
	public:
		~copy_context();
		void do_copy();
		size_t block_size=0;
		size_t input_offset=0;
		size_t output_offset=0;
		int in_fd=-1;
		int out_fd=-1;
		bool sync=false;
		bool use_copy_file_range=false;
		bool use_sendfile=false;
		copy_status stat{};
	private:
		copy_ret try_copy_file_range();
		copy_ret try_sendfile();
		copy_ret try_readwrite();
		size_t to_copy=0,written_cur=0;
		off_t off_in=0,off_out=0;
		void*buff=nullptr;
		size_t buff_size=0;
};
extern int helper_progress_main(int argc,char**argv);
extern int helper_status_main(int argc,char**argv);
extern int helper_write_main(int argc,char**argv);
#endif
