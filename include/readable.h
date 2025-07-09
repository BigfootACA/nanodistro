#ifndef READABLE_H
#define READABLE_H
#include<cstdint>
#include<cstddef>
#include<string>
extern const char*size_units_t[];
extern const char*size_units_ts[];
extern const char*size_units_b[];
extern const char*size_units_bps[];
extern const char*size_units_ib[];
extern const char*size_units_ibs[];
extern const char*size_units_hz[];
extern const char*format_size_ex(char*buf,size_t len,uint64_t val,const char**units,size_t blk);
extern const char*format_size_float_ex(char*buf,size_t len,uint64_t val,const char**units,size_t blk,uint8_t dot);
extern std::string format_size(uint64_t val,const char**units=size_units_ib,size_t blk=1024);
extern std::string format_size_float(uint64_t val,const char**units=size_units_ib,size_t blk=1024,uint8_t dot=2);
extern std::string format_size_hz(uint64_t val);
extern std::string format_size_float_hz(uint64_t val,uint8_t dot);
extern std::string size_string(uint64_t val);
extern std::string size_string_float(uint64_t val);
#endif
