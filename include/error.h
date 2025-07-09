#ifndef ERROR_H
#define ERROR_H
#include"log.h"
#include<source_location>
class RuntimeErrorImpl:public std::exception{
	public:
		RuntimeErrorImpl(const std::string&msg,const std::source_location&loc);
		RuntimeErrorImpl(const std::string&msg,const log_location&loc);
		const char*what()const noexcept override;
		std::string msg{};
		log_location loc{};
	protected:
		std::string vmsg{};
};
class InvalidArgumentImpl:public RuntimeErrorImpl{
	public:
		InvalidArgumentImpl(const std::string&msg,const std::source_location&loc);
		InvalidArgumentImpl(const std::string&msg,const log_location&loc);
};
class ErrnoErrorImpl:public RuntimeErrorImpl{
	public:
		ErrnoErrorImpl(int e,const std::string&msg,const std::source_location&loc);
		ErrnoErrorImpl(int e,const std::string&msg,const log_location&loc);
		int err=0;
};
#define RuntimeError(...)\
	RuntimeErrorImpl(\
		std::format(__VA_ARGS__),\
		std::source_location::current()\
	)
#define ErrnoErrorWith(err,...)\
	ErrnoErrorImpl(\
		err,\
		std::format(__VA_ARGS__),\
		std::source_location::current()\
	)
#define ErrnoError(...) ErrnoErrorWith(errno,__VA_ARGS__)
#define InvalidArgument(...)\
	InvalidArgumentImpl(\
		std::format(__VA_ARGS__),\
		std::source_location::current()\
	)
#endif
