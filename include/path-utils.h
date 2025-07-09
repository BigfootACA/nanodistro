#ifndef PATH_UTILS_H
#define PATH_UTILS_H
#include<string>
extern std::string path_basename(const std::string&str);
extern std::string path_dirname(const std::string&str);
extern std::string path_join(const std::string&dir,const std::string&file);
extern std::string path_find_exec(const std::string&exe);
extern std::string path_get_self();
#endif
