#include<cstdint>
#include<cstdlib>
#include<cstddef>
#include<cstring>
#include<cstdio>
#include<cinttypes>
#include<format>
#include"readable.h"

const char*size_units_t[]={"T","KT","MT","GT","TT","PT","ET","ZT","YT",nullptr};
const char*size_units_ts[]={"T/s","KT/s","MT/s","GT/s","T/sT/s","PT/s","ET/s","ZT/s","YT/s",nullptr};
const char*size_units_b[]={"B","KB","MB","GB","TB","PB","EB","ZB","YB",nullptr};
const char*size_units_bps[]={"bps","Kbps","Mbps","Gbps","Tbps","Pbps","Ebps","Zbps","Ybps",nullptr};
const char*size_units_ib[]={"B","KiB","MiB","GiB","TiB","PiB","EiB","ZiB","YiB",nullptr};
const char*size_units_ibs[]={"B/s","KiB/s","MiB/s","GiB/s","TiB/s","PiB/s","EiB/s","ZiB/s","YiB/s",nullptr};
const char*size_units_hz[]={"Hz","KHz","MHz","GHz","THz","PHz","EHz","ZHz","YHz",nullptr};

const char*format_size_ex(
	char*buf,size_t len,
	uint64_t val,const char**units,
	size_t blk
){
	int unit=0;
	if(!buf||len<=0||!units)return nullptr;
	memset(buf,0,len);
	if(val==0)return strncpy(buf,"0",len-1);
	while((val>=blk)&&units[unit+1])val/=blk,unit++;
	snprintf(buf,len-1,"%" PRIu64 " %s",val,units[unit]);
	return buf;
}

const char*format_size_float_ex(
	char*buf,size_t len,
	uint64_t val,const char**units,
	size_t blk,uint8_t dot
){
	int unit=0;
	uint64_t left,right,pd=10;
	if(!buf||len<=0||!units)return nullptr;
	if(dot==0)return format_size_ex(buf,len,val,units,blk);
	for(;dot>0;dot--)pd*=10;
	memset(buf,0,len);
	if(val==0)return strncpy(buf,"0",len);
	while((val>=blk*blk)&&units[unit+1])val/=blk,unit++;
	left=val,right=0;
	if(val>=blk&&units[unit+1])left=val/blk,right=(val%blk)*pd/blk,unit++;
	if(right%10>=5)right+=10;
	right/=10;
	while(right>0&&(right%10)==0)right/=10;
	snprintf(buf,len,"%" PRIu64 ".%" PRIu64 " %s",left,right,units[unit]);
	return buf;
}

std::string format_size(uint64_t val,const char**units,size_t blk){
	char buff[256];
	if(!format_size_ex(buff,sizeof(buff),val,units,blk))return "";
	return buff;
}

std::string format_size_float(uint64_t val,const char**units,size_t blk,uint8_t dot){
	char buff[256];
	if(!format_size_float_ex(buff,sizeof(buff),val,units,blk,dot))return "";
	return buff;
}

std::string format_size_hz(uint64_t val){
	return format_size(val,size_units_hz,1000);
}

std::string format_size_float_hz(uint64_t val,uint8_t dot){
	return format_size_float(val,size_units_hz,1000,dot);
}

std::string size_string(uint64_t val){
	return std::format("{} ({} bytes)",format_size(val),val);
}

std::string size_string_float(uint64_t val){
	return std::format("{} ({} bytes)",format_size_float(val),val);
}
