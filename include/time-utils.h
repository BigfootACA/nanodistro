#ifndef TIME_UTILS_H
#define TIME_UTILS_H
#include<ctime>
#include<cstdint>
#include<string>
#include<sys/time.h>
#define DEFAULT_TIME_FMT "%Y/%m/%d %H:%M:%S"
#define DEFAULT_TIME_FMT_TZ DEFAULT_TIME_FMT " %z"
class timestamp{
	public:
		enum unit{
			year,
			month,
			week,
			day,
			hour,
			second,
			millisecond,
			microsecond,
			nanosecond,
		};
		inline timestamp(){}
		inline timestamp(const timeval&tv):ts{tv.tv_sec,tv.tv_usec*1000}{}
		inline timestamp(const timespec&ts):ts(ts){}
		timestamp(int year,int month,int day,int hour,int minute,int second);
		timestamp(const std::string&str,const std::string&fmt,size_t&parsed);
		timestamp(const std::string&str,const std::string&fmt=DEFAULT_TIME_FMT);
		timestamp(const tm&tm);
		timestamp(int64_t val,unit unit=unit::second);
		timestamp(uint64_t val,unit unit=unit::second);
		timestamp(int32_t val,unit unit=unit::second);
		timestamp(uint32_t val,unit unit=unit::second);
		timestamp(double val,unit unit=unit::second);
		timestamp&add(const timestamp&t);
		timestamp&sub(const timestamp&t);
		timestamp&diff(const timestamp&t);
		int64_t tonumber(unit unit)const;
		double todouble(unit unit)const;
		tm tolocal()const;
		tm toutc()const;
		inline int64_t toyear()const{return tonumber(year);}
		inline int64_t tomonth()const{return tonumber(month);}
		inline int64_t toweek()const{return tonumber(week);}
		inline int64_t today()const{return tonumber(day);}
		inline int64_t tohour()const{return tonumber(hour);}
		inline int64_t tosecond()const{return tonumber(second);}
		inline int64_t tomillisecond()const{return tonumber(millisecond);}
		inline int64_t tomicrosecond()const{return tonumber(microsecond);}
		inline int64_t tonanosecond()const{return tonumber(nanosecond);}
		std::string format_utc(const std::string&fmt=DEFAULT_TIME_FMT)const;
		std::string format_local(const std::string&fmt=DEFAULT_TIME_FMT)const;
		inline timespec totimespec()const{return ts;}
		inline timeval totimeval()const{return {ts.tv_sec,ts.tv_nsec/1000};}
		inline time_t totime()const{return tonumber(unit::second);}
		inline operator bool()const{return ts.tv_sec!=0||ts.tv_nsec!=0;}
		inline operator time_t()const{return totime();}
		inline operator timeval()const{return totimeval();}
		inline operator timespec()const{return totimespec();}
		inline operator tm()const{return tolocal();}
		inline operator double()const{return todouble(unit::second);}
		inline operator std::string()const{return format_local();}
		inline timestamp operator +(const timestamp&ts)const{return timestamp(*this).add(ts);}
		inline timestamp operator +(const timeval&tv)const{return timestamp(*this).add(timestamp(tv));}
		inline timestamp operator +(const timespec&ts)const{return timestamp(*this).add(timestamp(ts));}
		inline timestamp operator +(const tm&tm)const{return timestamp(*this).add(timestamp(tm));}
		inline timestamp operator +(int64_t v)const{return timestamp(*this).add(timestamp(v,second));}
		inline timestamp operator +(int32_t v)const{return timestamp(*this).add(timestamp(v,second));}
		inline timestamp operator +(uint64_t v)const{return timestamp(*this).add(timestamp(v,second));}
		inline timestamp operator +(uint32_t v)const{return timestamp(*this).add(timestamp(v,second));}
		inline timestamp operator +(double v)const{return timestamp(*this).add(timestamp(v,second));}
		inline timestamp operator -(const timestamp&ts)const{return timestamp(*this).sub(ts);}
		inline timestamp operator -(const timeval&tv)const{return timestamp(*this).sub(timestamp(tv));}
		inline timestamp operator -(const timespec&ts)const{return timestamp(*this).sub(timestamp(ts));}
		inline timestamp operator -(const tm&tm)const{return timestamp(*this).sub(timestamp(tm));}
		inline timestamp operator -(int64_t v)const{return timestamp(*this).sub(timestamp(v,second));}
		inline timestamp operator -(int32_t v)const{return timestamp(*this).sub(timestamp(v,second));}
		inline timestamp operator -(uint64_t v)const{return timestamp(*this).sub(timestamp(v,second));}
		inline timestamp operator -(uint32_t v)const{return timestamp(*this).sub(timestamp(v,second));}
		inline timestamp operator -(double v)const{return timestamp(*this).sub(timestamp(v,second));}
		inline timestamp&operator =(const timeval&tv){ts.tv_sec=tv.tv_sec,ts.tv_nsec=tv.tv_usec*1000;return*this;}
		inline timestamp&operator =(const timespec&ts){this->ts=ts;return*this;}
		inline timestamp&operator =(const tm&tm){return*this=timestamp(tm);}
		inline timestamp&operator =(int64_t v){return*this=timestamp(v,second);}
		inline timestamp&operator =(int32_t v){return*this=timestamp(v,second);}
		inline timestamp&operator =(uint64_t v){return*this=timestamp(v,second);}
		inline timestamp&operator =(uint32_t v){return*this=timestamp(v,second);}
		inline timestamp&operator =(double v){return*this=timestamp(v,second);}
		inline timestamp&operator +=(const timestamp&ts){return add(ts);}
		inline timestamp&operator +=(const timeval&tv){return add(timestamp(tv));}
		inline timestamp&operator +=(const timespec&ts){return add(timestamp(ts));}
		inline timestamp&operator +=(const tm&tm){return add(timestamp(tm));}
		inline timestamp&operator +=(int64_t v){return add(timestamp(v,second));}
		inline timestamp&operator +=(int32_t v){return add(timestamp(v,second));}
		inline timestamp&operator +=(uint64_t v){return add(timestamp(v,second));}
		inline timestamp&operator +=(uint32_t v){return add(timestamp(v,second));}
		inline timestamp&operator +=(double v){return add(timestamp(v,second));}
		inline timestamp&operator -=(const timestamp&ts){return sub(ts);}
		inline timestamp&operator -=(const timeval&tv){return sub(timestamp(tv));}
		inline timestamp&operator -=(const timespec&ts){return sub(timestamp(ts));}
		inline timestamp&operator -=(const tm&tm){return sub(timestamp(tm));}
		inline timestamp&operator -=(int64_t v){return sub(timestamp(v,second));}
		inline timestamp&operator -=(int32_t v){return sub(timestamp(v,second));}
		inline timestamp&operator -=(uint64_t v){return sub(timestamp(v,second));}
		inline timestamp&operator -=(uint32_t v){return sub(timestamp(v,second));}
		inline timestamp&operator -=(double v){return sub(timestamp(v,second));}
		inline bool operator ==(const timestamp&t)const{return ts.tv_sec==t.ts.tv_sec&&ts.tv_nsec==t.ts.tv_nsec;}
		inline bool operator !=(const timestamp&t)const{return ts.tv_sec!=t.ts.tv_sec||ts.tv_nsec!=t.ts.tv_nsec;}
		inline bool operator <(const timestamp&t)const{return ts.tv_sec<t.ts.tv_sec||(ts.tv_sec==t.ts.tv_sec&&ts.tv_nsec<t.ts.tv_nsec);}
		inline bool operator <=(const timestamp&t)const{return ts.tv_sec<t.ts.tv_sec||(ts.tv_sec==t.ts.tv_sec&&ts.tv_nsec<=t.ts.tv_nsec);}
		inline bool operator >(const timestamp&t)const{return ts.tv_sec>t.ts.tv_sec||(ts.tv_sec==t.ts.tv_sec&&ts.tv_nsec>t.ts.tv_nsec);}
		inline bool operator >=(const timestamp&t)const{return ts.tv_sec>t.ts.tv_sec||(ts.tv_sec==t.ts.tv_sec&&ts.tv_nsec>=t.ts.tv_nsec);}
		inline bool operator ==(const timeval&tv)const{return *this==timestamp(tv);}
		inline bool operator !=(const timeval&tv)const{return *this!=timestamp(tv);}
		inline bool operator <(const timeval&tv)const{return *this<timestamp(tv);}
		inline bool operator <=(const timeval&tv)const{return *this<=timestamp(tv);}
		inline bool operator >(const timeval&tv)const{return *this>timestamp(tv);}
		inline bool operator >=(const timeval&tv)const{return *this>=timestamp(tv);}
		inline bool operator ==(const timespec&ts)const{return *this==timestamp(ts);}
		inline bool operator !=(const timespec&ts)const{return *this!=timestamp(ts);}
		inline bool operator <(const timespec&ts)const{return *this<timestamp(ts);}
		inline bool operator <=(const timespec&ts)const{return *this<=timestamp(ts);}
		inline bool operator >(const timespec&ts)const{return *this>timestamp(ts);}
		inline bool operator >=(const timespec&ts)const{return *this>=timestamp(ts);}
		inline bool operator ==(const tm&tm)const{return *this==timestamp(tm);}
		inline bool operator !=(const tm&tm)const{return *this!=timestamp(tm);}
		inline bool operator <(const tm&tm)const{return *this<timestamp(tm);}
		inline bool operator <=(const tm&tm)const{return *this<=timestamp(tm);}
		inline bool operator >(const tm&tm)const{return *this>timestamp(tm);}
		inline bool operator >=(const tm&tm)const{return *this>=timestamp(tm);}
		inline bool operator ==(int64_t v)const{return *this==timestamp(v,second);}
		inline bool operator !=(int64_t v)const{return *this!=timestamp(v,second);}
		inline bool operator <(int64_t v)const{return *this<timestamp(v,second);}
		inline bool operator <=(int64_t v)const{return *this<=timestamp(v,second);}
		inline bool operator >(int64_t v)const{return *this>timestamp(v,second);}
		inline bool operator >=(int64_t v)const{return *this>=timestamp(v,second);}
		inline bool operator ==(int32_t v)const{return *this==timestamp(v,second);}
		inline bool operator !=(int32_t v)const{return *this!=timestamp(v,second);}
		inline bool operator <(int32_t v)const{return *this<timestamp(v,second);}
		inline bool operator <=(int32_t v)const{return *this<=timestamp(v,second);}
		inline bool operator >(int32_t v)const{return *this>timestamp(v,second);}
		inline bool operator >=(int32_t v)const{return *this>=timestamp(v,second);}
		inline bool operator ==(uint64_t v)const{return *this==timestamp(v,second);}
		inline bool operator !=(uint64_t v)const{return *this!=timestamp(v,second);}
		inline bool operator <(uint64_t v)const{return *this<timestamp(v,second);}
		inline bool operator <=(uint64_t v)const{return *this<=timestamp(v,second);}
		inline bool operator >(uint64_t v)const{return *this>timestamp(v,second);}
		inline bool operator >=(uint64_t v)const{return *this>=timestamp(v,second);}
		inline bool operator ==(uint32_t v)const{return *this==timestamp(v,second);}
		inline bool operator !=(uint32_t v)const{return *this!=timestamp(v,second);}
		inline bool operator <(uint32_t v)const{return *this<timestamp(v,second);}
		inline bool operator <=(uint32_t v)const{return *this<=timestamp(v,second);}
		inline bool operator >(uint32_t v)const{return *this>timestamp(v,second);}
		inline bool operator >=(uint32_t v)const{return *this>=timestamp(v,second);}
		inline bool operator ==(double v)const{return *this==timestamp(v,second);}
		inline bool operator !=(double v)const{return *this!=timestamp(v,second);}
		inline bool operator <(double v)const{return *this<timestamp(v,second);}
		inline bool operator <=(double v)const{return *this<=timestamp(v,second);}
		inline bool operator >(double v)const{return *this>timestamp(v,second);}
		inline bool operator >=(double v)const{return *this>=timestamp(v,second);}
		void sleep();
		void sleepi();
		static inline int64_t convert(int64_t val,unit from,unit to){return timestamp(val,from).tonumber(to);}
		static inline double convert(double val,unit from,unit to){return timestamp(val,from).tonumber(to);}
		static timestamp get(clock_t clk_id=CLOCK_REALTIME);
		static void sleep(const timestamp&t);
		static void sleepi(const timestamp&t);
		static inline timestamp now(){return get();}
		static inline timestamp boottime(){return get(CLOCK_BOOTTIME);}
		static inline timestamp monotonic(){return get(CLOCK_MONOTONIC);}
		static int parse(const std::string&str,const std::string&fmt=DEFAULT_TIME_FMT);
	private:
		void fix();
		timespec ts{};
};
#endif
