#include "types.h"

// 互斥锁
struct spinlock {
    uint locked;    // 非零表示持有

    // 调试信息
    char *name;  
    int cpuid;
};