#define _FILE_OFFSET_BITS 64
#include<sys/sendfile.h>
#include<unistd.h>
#include"internal.h"
#include"log.h"
#include"error.h"

copy_ret copy_context::try_copy_file_range(){
	auto ret=copy_file_range(
		in_fd,&off_in,out_fd,&off_out,to_copy,0
	);
	if(ret<0){
		switch(errno){
			case EINTR:
			case EAGAIN:
				return COPY_CONTINUE;
			case EINVAL:
			case EPERM:
			case ENOSYS:
			case ENOTSUP:
			case EXDEV:
				use_copy_file_range=false;
				log_warning("copy_file_range unavailable, fallback to sendfile");
				return COPY_FALLBACK;
		}
		throw ErrnoError(
			"copy_file_range at in {} out {} len {} failed",
			off_in,off_out,to_copy
		);
	}
	if(ret==0)return COPY_END;
	written_cur=ret;
	return COPY_CONTINUE;
};

copy_ret copy_context::try_sendfile(){
	lseek(out_fd,off_out,SEEK_SET);
	auto ret=sendfile(out_fd,in_fd,&off_in,to_copy);
	if(ret<0){
		switch(errno){
			case EINTR:
			case EAGAIN:
				return COPY_CONTINUE;
			case EINVAL:
			case ENOSYS:
				use_sendfile=false;
				log_warning("sendfile unavailable, fallback to read/write");
				return COPY_FALLBACK;
		}
		throw ErrnoError(
			"sendfile at in {} out {} len {} failed",
			off_in,off_out,to_copy
		);
	}
	if(ret==0)return COPY_END;
	written_cur=ret;
	return COPY_CONTINUE;
};

copy_ret copy_context::try_readwrite(){
	ssize_t ret;
	if(buff_size<to_copy){
		if(buff)free(buff);
		buff=malloc(to_copy);
		if(!buff)throw ErrnoError("malloc failed");
		buff_size=to_copy;
	}
	if(!buff)throw RuntimeError("buffer not allocated");
	ret=pread64(in_fd,buff,to_copy,off_in);
	if(ret<0){
		switch(errno){
			case EINTR:
			case EAGAIN:
				return COPY_CONTINUE;
		}
		throw ErrnoError(
			"read at in {} len {} failed",
			off_in,to_copy
		);
	}
	if(ret==0)return COPY_END;
	ret=pwrite64(out_fd,buff,ret,off_out);
	if(ret<0){
		switch(errno){
			case EINTR:
			case EAGAIN:
				return COPY_CONTINUE;
		}
		throw ErrnoError(
			"write at out {} len {} failed",
			off_out,to_copy
		);
	}
	if(ret==0)return COPY_END;
	written_cur=ret;
	return COPY_CONTINUE;
};

void copy_context::do_copy(){
	copy_ret s;
	while(stat.cur.written<stat.total){
		to_copy=std::min(block_size,stat.total-stat.cur.written);
		off_in=input_offset+stat.cur.written;
		off_out=output_offset+stat.cur.written;
		written_cur=0,s=COPY_FALLBACK;
		if(s==COPY_FALLBACK&&use_copy_file_range)
			s=try_copy_file_range();
		if(s==COPY_FALLBACK&&use_sendfile)
			s=try_sendfile();
		if(s==COPY_FALLBACK)
			s=try_readwrite();
		if(s==COPY_FALLBACK)
			throw RuntimeError("no copy method available");
		if(written_cur>0){
			if(sync)fsync(out_fd);
			auto now=timestamp::now();
			std::lock_guard<std::mutex>lock(stat.lock);
			stat.elast=stat.cur;
			stat.cur.written+=written_cur;
			stat.cur.time=now;
		}
		if(s==COPY_END)break;
		if(s==COPY_CONTINUE)continue;
		throw RuntimeError("unexpected copy status");
	}
}

copy_context::~copy_context(){
	if(buff)free(buff);
	buff=nullptr;
	buff_size=0;	
}

void copy_status::calc(){
	auto passed=cur.time-elast.time;
	if(passed.tosecond()<1){
		progress.passed=cur.time-clast.time;
		progress.written=cur.written-clast.written;
	}else{
		progress.passed=passed;
		progress.written=cur.written-elast.written;
	}
	auto passed_ms=progress.passed.tomillisecond();
	progress.speed=passed_ms>0?progress.written*1000/passed_ms:0;
	progress.percent=total>0?std::min<size_t>(cur.written*100/total,100):100;
	clast=cur;
}
