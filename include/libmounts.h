#ifndef LIBMOUNTS_H
#define LIBMOUNTS_H
#include<string>
#include<libmount/libmount.h>
extern std::string libmount_strerror(int err);
#endif
