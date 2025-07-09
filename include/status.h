#ifndef STATUS_H
#define STATUS_H
#include<string>
extern int installer_status_fd;
extern int installer_init();
extern void installer_write_cmd(const std::string&cmd);
extern void installer_set_progress_enable(bool enable);
extern void installer_set_progress_value(int value);
extern void installer_set_status(const std::string&value);
extern void installer_load_context(const std::string&path="");
#endif
