#include<sys/time.h>
#include"time-utils.h"
#include"error.h"

timestamp::timestamp(const tm&t){
	tm tx=t;
	ts.tv_sec=mktime(&tx);
	if(ts.tv_sec==-1)
		throw InvalidArgument("invalid timestamp structure");
	ts.tv_nsec=0;
}

timestamp::timestamp(int year,int month,int day,int hour,int minute,int second){
	tm tx{};
	tx.tm_year=year-1900;
	tx.tm_mon=month-1;
	tx.tm_mday=day;
	tx.tm_hour=hour;
	tx.tm_min=minute;
	tx.tm_sec=second;
	*this=timestamp(tx);
}

timestamp::timestamp(const std::string&str,const std::string&fmt,size_t&parsed){
	tm time{};
	parsed=0;
	auto ret=strptime(str.c_str(),fmt.c_str(),&time);
	if(ret){
		ts.tv_sec=mktime(&time);
		ts.tv_nsec=0;
		parsed=ret-str.c_str();
	}
}

timestamp::timestamp(const std::string&str,const std::string&fmt){
	size_t parsed;
	*this=timestamp(str,fmt,parsed);
	if(parsed!=str.length())throw InvalidArgument("invalid timestamp string {}",str);
}

template<typename T>
static void num_convert(T val,timestamp::unit unit,timespec&ts){
	switch(unit){
		case timestamp::year:ts.tv_sec=val*365*24*60*60;ts.tv_nsec=0;break;
		case timestamp::month:ts.tv_sec=val*30*24*60*60;ts.tv_nsec=0;break;
		case timestamp::week:ts.tv_sec=val*7*24*60*60;ts.tv_nsec=0;break;
		case timestamp::day:ts.tv_sec=val*24*60*60;ts.tv_nsec=0;break;
		case timestamp::hour:ts.tv_sec=val*60*60;ts.tv_nsec=0;break;
		case timestamp::second:ts.tv_sec=val;ts.tv_nsec=0;break;
		case timestamp::millisecond:ts.tv_sec=val/1000;ts.tv_nsec=(val%1000+1000)%1000*1000000;break;
		case timestamp::microsecond:ts.tv_sec=val/1000000;ts.tv_nsec=(val%1000000)*1000;break;
		case timestamp::nanosecond:ts.tv_sec=val/1000000000;ts.tv_nsec=val%1000000000;break;
	}
}

timestamp::timestamp(int32_t val,unit unit){num_convert(val,unit,ts);}
timestamp::timestamp(int64_t val,unit unit){num_convert(val,unit,ts);}
timestamp::timestamp(uint32_t val,unit unit){num_convert(val,unit,ts);}
timestamp::timestamp(uint64_t val,unit unit){num_convert(val,unit,ts);}

timestamp::timestamp(double val,unit unit){
	switch(unit){
		case unit::year:ts.tv_sec=(int64_t)val*365*24*60*60;ts.tv_nsec=(val-(int64_t)val)*1000000000;break;
		case unit::month:ts.tv_sec=(int64_t)val*30*24*60*60;ts.tv_nsec=(val-(int64_t)val)*1000000000;break;
		case unit::week:ts.tv_sec=(int64_t)val*7*24*60*60;ts.tv_nsec=(val-(int64_t)val)*1000000000;break;
		case unit::day:ts.tv_sec=(int64_t)val*24*60*60;ts.tv_nsec=(val-(int64_t)val)*1000000000;break;
		case unit::hour:ts.tv_sec=(int64_t)val*60*60;ts.tv_nsec=(val-(int64_t)val)*1000000000;break;
		case unit::second:ts.tv_sec=(int64_t)val;ts.tv_nsec=(val-(int64_t)val)*1000000000;break;
		case unit::millisecond:ts.tv_sec=(int64_t)val/1000;ts.tv_nsec=((int64_t)val%1000)*1000000;break;
		case unit::microsecond:ts.tv_sec=(int64_t)val/1000000;ts.tv_nsec=((int64_t)val%1000000)*1000;break;
		case unit::nanosecond:ts.tv_sec=(int64_t)val/1000000000;ts.tv_nsec=(int64_t)val%1000000000;break;
	}
}

void timestamp::fix(){
	if(ts.tv_nsec<0){
		ts.tv_sec-=(1000000000ll-ts.tv_nsec)/1000000000ll;
		ts.tv_nsec=(ts.tv_nsec%1000000000ll+1000000000ll)%1000000000ll;
	}else if(ts.tv_nsec>=1000000000ll){
		ts.tv_sec+=ts.tv_nsec/1000000000ll;
		ts.tv_nsec%=1000000000ll;
	}
}

timestamp&timestamp::add(const timestamp&t){
	ts.tv_sec+=t.ts.tv_sec;
	ts.tv_nsec+=t.ts.tv_nsec;
	fix();
	return *this;
}

timestamp&timestamp::sub(const timestamp&t){
	ts.tv_sec-=t.ts.tv_sec;
	ts.tv_nsec-=t.ts.tv_nsec;
	fix();
	return *this;
}

timestamp&timestamp::diff(const timestamp&t){
	ts.tv_sec=ts.tv_sec-t.ts.tv_sec;
	ts.tv_nsec=ts.tv_nsec-t.ts.tv_nsec;
	fix();
	return *this;
}

int64_t timestamp::tonumber(unit unit)const{
	switch(unit){
		case unit::year:return ts.tv_sec/(365*24*60*60);
		case unit::month:return ts.tv_sec/(30*24*60*60);
		case unit::week:return ts.tv_sec/(7*24*60*60);
		case unit::day:return ts.tv_sec/(24*60*60);
		case unit::hour:return ts.tv_sec/(60*60);
		case unit::second:return ts.tv_sec;
		case unit::millisecond:return ts.tv_sec*1000+ts.tv_nsec/1000000;
		case unit::microsecond:return ts.tv_sec*1000000+ts.tv_nsec/1000;
		case unit::nanosecond:return ts.tv_sec*1000000000+ts.tv_nsec;
	}
	return 0;
}

double timestamp::todouble(unit unit)const{
	switch(unit){
		case unit::year:return (double)ts.tv_sec/(365*24*60*60)+(double)ts.tv_nsec/1000000000;
		case unit::month:return (double)ts.tv_sec/(30*24*60*60)+(double)ts.tv_nsec/1000000000;
		case unit::week:return (double)ts.tv_sec/(7*24*60*60)+(double)ts.tv_nsec/1000000000;
		case unit::day:return (double)ts.tv_sec/(24*60*60)+(double)ts.tv_nsec/1000000000;
		case unit::hour:return (double)ts.tv_sec/(60*60)+(double)ts.tv_nsec/1000000000;
		case unit::second:return (double)ts.tv_sec+(double)ts.tv_nsec/1000000000;
		case unit::millisecond:return ts.tv_sec*1000+(double)ts.tv_nsec/1000000;
		case unit::microsecond:return ts.tv_sec*1000000+(double)ts.tv_nsec/1000;
		case unit::nanosecond:return ts.tv_sec*1000000000+(double)ts.tv_nsec;
	}
	return 0;
}

tm timestamp::tolocal()const{
	tm tx{};
	localtime_r(&ts.tv_sec,&tx);
	return tx;
}

tm timestamp::toutc()const{
	tm tx{};
	gmtime_r(&ts.tv_sec,&tx);
	return tx;
}

timestamp timestamp::get(clock_t clk_id){
	timespec ts{};
	auto r=clock_gettime(clk_id,&ts);
	if(r<0)throw ErrnoError("clock_gettime failed");
	return ts;
}

void timestamp::sleep(const timestamp&t){
	timespec v=t.ts;
	int r;
	do{
		r=nanosleep(&v,&v);
	}while(r<0&&errno==EINTR);
}

void timestamp::sleepi(const timestamp&t){
	timespec v=t.ts;
	nanosleep(&v,nullptr);
}

void timestamp::sleep(){
	sleep(*this);
}

void timestamp::sleepi(){
	sleepi(*this);
}

std::string timestamp::format_utc(const std::string&fmt)const{
	auto t=toutc();
	char buff[1024]{};
	auto ret=strftime(buff,sizeof(buff),fmt.c_str(),&t);
	return std::string(buff,buff+std::min(sizeof(buff)-1,ret));
}

std::string timestamp::format_local(const std::string&fmt)const{
	auto t=tolocal();
	char buff[1024]{};
	auto ret=strftime(buff,sizeof(buff),fmt.c_str(),&t);
	return std::string(buff,buff+std::min(sizeof(buff)-1,ret));
}
