#include "defs.h"
#include "memlayout.h"
#include "param.h"
#include "spinlock.h" 
#include "types.h"

extern char end[]; // from kernel.ld

static struct run {
    struct run *next;
};

static struct {
    struct spinlock lock;
    struct run *freelist;
} kmem;

// 添加一页进入空闲页表
void kfreee(void *pa) {
    struct run *r;
    if((uint64)pa % PGSIZE != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP) {
        panic("kfree");
    }
    memset(pa, 1, PGSIZE);
    acquire(&kmem.lock);
    r = (struct run *)pa;
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
}

// 添加一个范围进入空闲页表
void freerange(void *pa_start, void *pa_end) {
    char *p;
    p = (char *)PGROUNDUP((uint64)pa_start);
    for(;p + PGSIZE <= (char *)pa_end; p += PGSIZE) {
        kfreee(p);
    }
}

// 添加 end 到 PHYSTOP 所有物理地址至空闲页表
void kinit(void) {
    initlock(&kmem.lock, "kmem");
    freerange(end, (void *)PHYSTOP);
}

// 取出空闲页表头部
// 返回 0 表示没有可分别配的空闲页
void *kalloc(void) {
    struct run *r;
    acquire(&kmem.lock);
    r = kmem.freelist;
    if(r) {
        kmem.freelist = r->next;
    }
    release(&kmem.lock);
    if(r) {
        memset((char *)r, 5, PGSIZE);
    }
    return (void *)r;
}