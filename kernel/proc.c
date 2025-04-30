#include "defs.h"
#include "param.h"
#include "proc.h"
#include "riscv.h"
#include "spinlock.h"

struct cpu cpus[NCPU];
struct proc procs[NPROC];

extern void swtch(); // in swtch.S

// 需要在禁止中断状态下获取处理器编号
int cpuid(void) {
    int id = r_tp();
    return id;
}

// 需要在禁止中断状态下获取处理器信息
struct cpu *mycpu(void) {
    int id;
    struct cpu *c;

    id = cpuid();
    c = &cpus[id];
    return c;
}

// 进程信息
struct proc *myproc(void) {
    struct cpu *c;
    struct proc *p;

    push_off();
    c = mycpu();
    p = c->proc;
    pop_off();
    return p;
}

// 操作系统调度器
// 每个CPU初始化完成后跳转到此
// 该程序不会返回，一直循环进行：
// 1、选择一个可执行进程
// 2、进行上下文切换 swtch()
// 3、最终进程结束转移控制权
// 4、通过 swtch() 返回此处
void scheduler(void) {
    struct cpu *c = mycpu();
    struct proc *p;
    int found;

    for(;;) {
        // 开启中断避免死锁
        intr_on();

        found = 0;
        for(p = procs; p < &procs[NPROC]; p++) {
            acquire(&p->lock);
            if(p->state == RUNNABLE) {
                // 转换到选择的进程
                p->state = RUNNING;
                c->proc = p;
                swtch(&c->context, &p->context);

                // 进程工作完成
                c->proc = 0;
                found = 1;
            }
            release(&p->lock);
        }
        if(found == 0) {
            // 如果没有找到可运行的进程
            // 中断处理程序可能会关中断
            // 此时需要打开中断并进行低功耗等待模式
            intr_on();
            asm volatile("wfi");
        }
    }
}

// 进程从工作状态到离开处理器
// 必须在持有进程锁的状态下切换
static void sched(void) {
    struct cpu *c = mycpu();
    int interna = c->interna;
    struct proc *p = myproc();

    if(intr_get()) {
        panic("sched: interruptible");
    }
    if(c->noff != 1) {
        panic("sched: two much off");
    }
    if(!holding(&p->lock)) {
        panic("sched: no proc locks");
    }
    if(p->state == RUNNING) {
        panic("sched: still running");
    }
    
    swtch(&p->context, &c->context);
    c->interna = interna;
}

// 进程主动放弃 CPU 控制权
void yield(void) {
    struct proc *p = myproc();
    acquire(&p->lock);
    p->state = RUNNABLE;
    sched();
    release(&p->lock);
}

// 进程因等待某一个共享资源而放弃 CPU
// 参数 lk 表示该进程所等待的资源锁
void sleep(void *chan, struct spinlock *lk) {
    struct proc *p = myproc();

    // 修改进程信息
    acquire(&p->lock);
    p->state = SLEEPING;
    p->chan = chan;

    // 放弃锁
    release(&lk);
    sched();

    p->chan = 0;
    release(&p->lock);
    acquire(lk);
}

void wakeup(void *chan) {
    struct proc *p;

    for(p = procs; p < &procs[NPROC]; p++) {
        // 确定不是当前进程防止死锁
        if(p != myproc()) {
            acquire(&p->lock);
            if(p->state == SLEEPING && p->chan == chan) {
                p->state = RUNNABLE;
            }
            release(&p->lock);
        }
    }
}

