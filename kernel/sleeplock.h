#include "spinlock.h"
#include "types.h"

// 进程长期锁
struct sleeplock {
    struct spinlock lk;

    uint locked;
    char *name;
    int pid;
};
