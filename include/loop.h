#ifndef LOOP_H
#define LOOP_H
#include<string>
extern int loop_get_free();
extern std::string loop_find_device(int loop_id=-1);
extern int loop_set_fd(int file_fd,int loop_id=-1);
extern int loop_set_file(const std::string&file,int loop_id=-1);
extern void loop_detach(int loop_id=-1);
#endif
