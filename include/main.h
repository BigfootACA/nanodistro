#ifndef MAIN_H
#define MAIN_H
#include<vector>
#include<string>
extern int locale_init();
extern int nanodistro_main(int argc,char**argv);
extern int netblk_main(int argc,char**argv);
extern int flasher_main(int argc,char**argv);
extern int helper_main(int argc,char**argv);
extern int main(int argc,char**argv);
extern std::vector<std::string>args;
#endif
