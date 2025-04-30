#include "riscv.h"
#include "spinlock.h"
#include "types.h"

// 进程上下文包含的寄存器信息
struct context {
    uint64 ra; // 返回地址
    uint64 sp; // 栈顶地址

    // 被调用者保存
    uint64 s0;
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
};

struct cpu {
    int cpuid;
    struct proc *proc;
    struct context context; 
    int noff;               // push_off() 嵌套深度
    int interna;            // push_off() 之前是否启用中断
};



struct proc {
    struct spinlock lock;

    // 需要在持有锁状态下修改信息
    enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
    enum procstate state;           // 进程状态
    void *chan;                     // 非零表示进程在等待队列中
    int killed;                     // 非零表示进程已经被杀死
    int xstate;                     // 返回给父进程的退出状态
    int pid;                        // 进程编号

    // 需要在持有 wait_lock 状态下修改信息
    struct proc *parent;            // 父进程

    // 进程的私有信息
    uint64 kstack;                  // 内核栈顶虚拟地址
    uint64 sz;                      // 进程内存大小（字节
    pagetable_t pagetable;          // 用户页表
    struct trapframe *trapframe;    // 数据页 trampoline.S
    struct context context;         // 进程上下文 swtch.S
    struct file *ofile[NOFILE];     // 打开文件
    struct inode *cwd;              // 当前目录
    char name[16];                  // 进程名
};

