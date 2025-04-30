#include "defs.h"
#include "proc.h"
#include "spinlock.h"
#include "riscv.h"

void initlock(struct spinlock *lk, char *name) {
    lk->locked = 0;
    lk->name = name;
    lk->cpuid = -1;
}

// 关闭中断并记录到结构体 cpu
void push_off(void) {
    int old;
    struct cpu *c;

    old = intr_get();
    intr_off();
    c = mycpu(); 
    if(c->noff == 0) {
        c->interna = old;
    }
    c->noff++;
}

// 恢复结构体 cpu 的中断记录
void pop_off(void) {
    struct cpu *c;

    if(intr_get()) {
        panic("pop_off: interruptible");
    }
    c = mycpu();
    if(c->noff < 1) {
        panic("pop_off: no noff");
    }
    c->noff--;
    if(c->noff == 0 && c->interna) {
        intr_on();
    }
}

int holding(struct spinlock *lk) {
    int r;
    r = (lk->locked & lk->cpuid == cpuid());
    return r;
}

// 进程持有锁的时期禁用中断，
// 防止中断处理程序获取锁导致死锁
void acquire(struct spinlock *lk) {
    push_off();
    if(holding(lk)) {
        panic("acquire");
    }

    // __sync_lock_test_and_set 表示原子比较交换指令：
    // a5 = 1
    // s1 = &(lk->locked)
    // amoswap.w.aq a5, a5, (s1)
    while(__sync_lock_test_and_set(&(lk->locked), 1) != 0) {
    }
    
    // 禁止编译器将加载和移动指令移动到该点之后
    __sync_synchronize();

    lk->cpuid = cpuid();
}


void release(struct spinlock *lk) {
    if(!holding(lk)) {
        panic("release");
    }

    lk->cpuid = -1;

    // 禁止编译器将加载和移动指令移动到该点之后
    __sync_synchronize();

    // __sync_lock_test_and_set 表示原子比较交换指令：
    // s1 = &lk->locked
    // amoswap.w zero, zero, (s1)
    __sync_lock_release(&(lk->locked));

    pop_off();
}


