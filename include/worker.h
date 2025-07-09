#ifndef WORKER_H
#define WORKER_H
#include<functional>
extern void worker_add(const std::function<void(void)>&cb);
extern void worker_init(int workers);
#endif
