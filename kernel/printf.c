//
// 格式化控制台输出 -- printf, panic.
//

// 操作可变参数的内建函数库
// 定义了 va_list, va_start(v,l), va_arg(v,l)
#include <stdarg.h>

#include "defs.h"
#include "spinlock.h"

volatile int panicked = 0;

static struct {
    struct spinlock lock;
    int locking; // 非零表示启用锁，保证程序崩溃不会占用输出通道
} pr;

void printfinit(void) {
    initlock(&pr.lock, "pr");
    pr.locking = 1;
}

int printf(char *fmt, ...) {
    va_list ap;
    int i, cx, c0, c1, c2, locking;
    char *s;

    locking = pr.locking;
    if(locking) {
        acquire(pr.lock);
    }


}


