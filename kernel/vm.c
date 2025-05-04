//
// 64位虚拟地址 va 的结构：
// 39...63 -- 必须为零
// 30...38 -- 9位二级索引
// 21...29 -- 9位一级索引
// 12...20 -- 9位零级索引
// 00...11 -- 12位页内偏移
// ##############################
// 64位页表项 PTE 的结构：
// 0        有效位 V
// 1...7    标志位 R,W,X,U,G,A,D
// 8...9    保留位
// 10...53  物理页号 PPN
// 54...63  保留位
// ##############################
// 56位物理地址结构：
// 0...11   OFFSET
// 12...55  PPN
//

#include "defs.h"
#include "riscv.h"

pagetable_t kernel_pagetable;
extern char etext[];        // from kernel.ld
extern char trampoline[];   // from trampoline.S

// 返回虚拟地址对应的页表项
// alloc != 0 则创建需要的页表项
// 寻找物理地址失败则返回 0
// 直接返回最底层的 PTE，调用者需需检查 PTE_V
static pte_t *walk(pagetable_t pagetable, uint64 va, int alloc) {
    pte_t *pte;
    if(va >= MAXVA) {
        panic("walk");
    }
    for(int level = 2; level > 0; level--) {
        pte = &pagetable[PX(level, va)];
        if(*pte & PTE_V) {
            pagetable = (pagetable_t)PTE2PA(*pte);
        } else {
            if(!alloc || (pagetable = (pagetable_t)kalloc()) == 0) {
                return 0;
            }
            memset((void *)pagetable, 0, PGSIZE);
            *pte = PA2PTE((uint64)pagetable) | PTE_V;
        }
    }
    return &pagetable[PX(0, va)];
}

// 递归删除所有自由页表项
void freewalk(pagetable_t pagetable) {
    pte_t pte;
    pagetable_t child;
    // 一个页表有 2^9=512 个页表项
    for(int i = 0; i < 512; i++) {
        pte = pagetable[i];
        // 若是非叶子节点项则递归并清除
        if((pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X) == 0)) {
            child = (pagetable_t)PTE2PA(pte);
            freewalk(child);
            pagetable[i] = 0;
        } else if(pte & PTE_V) {
            // 如果有未清除的叶子节点则陷入恐慌
            panic("freewalk");
        }
    }
    kfree((void *)pagetable);
}

// 返回虚拟地址对应的物理地址
// 只能返回已经存在的用户页
// 返回 0 表示不存在
uint64 walkaddr(pagetable_t pagetable, uint64 va) {
    pte_t *pte;
    uint pa;
    if(va >= MAXVA) {
        return 0;
    }
    pte = walk(pagetable, va, 0);
    if(pte == 0 || ((*pte & PTE_V) == 0) || ((*pte & PTE_V) == 0)) {
        return 0;
    }
    pa = PTE2PA(*pte);
    return pa;
}

// 创建页表项 PTEs
// perm 保存标识位
// 链接虚拟地址和物理地址
// 成功返回 0，失败返回 -1
int mappages(pagetable_t pagetable, uint64 va, uint64 pa, uint64 size, int perm) {
    pte_t *pte;
    if(size == 0) {
        panic("mappages: size zero");
    }
    if((va % PGSIZE) != 0) {
        panic("mappages: va not aligned");
    }
    if((size % PGSIZE) != 0) {
        panic("mappages: size not aligned");
    }
    for(uint64 a = va; a < va + size; a += PGSIZE, pa += PGSIZE) {
        if((pte = walk(pagetable, a, 1)) == 0) {
            return -1;
        }
        if(*pte & PTE_V) {
            panic("mappages: remap");
        }
        *pte = *pte | perm | PTE_V;
    }
}

// 为内核页表添加映射
void kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 size, int perm) {
    if(mappages(kpgtbl, va, pa, size, perm) == -1) {
        panic("kvmmap");
    }
}

// 创建一个空的页表
// 返回 0 表示内存已满
pagetable_t uvmcreate(void) {
    pagetable_t pagetable = (pagetable_t)kalloc();
    if(pagetable == 0) {
        return 0;
    }
    memset((void *)pagetable, 0, PGSIZE);
    return pagetable;
}

// 加载第一个用户 initcode 进程到页表地址 0 中，
// 该进程的大小 sz 必须小于一页
void uvmfirst(pagetable_t pagetable, uchar *src, uint sz) {
    char *mem;
    if(sz >= PGSIZE) {
        panic("uvmfirst");
    }
    mem = (char *)kalloc();
    memset(mem, 0, PGSIZE);
    mappages(pagetable, 0, (uint64)mem, PGSIZE, PTE_R | PTE_W | PTE_X | PTE_U);
    memmove((void *)mem, (void *)src, sz);
}

// 清除以 va 为起点数量为 npages 的页表项
// 规定 va 需要对齐页面
// 如果 do_free != 0 则将该页加入空闲链表
void uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free) {
    pte_t *pte;
    uint64 pa;
    if((va % PGSIZE) != 0) {
        panic("uvmunmap: va not aligned");
    }
    for(uint64 a = va; a < va + npages * PGSIZE; a += PGSIZE) {
        if((pte = walk(pagetable, a, 0)) == 0) {
            panic("uvmunmap: walk");
        }
        if((*pte & PTE_V) == 0) {
            panic("uvmunmap: not mapped");
        }
        if(PTE_FLAGS(*pte) == PTE_V) {
            panic("uvmunmap: not a leaf");
        }
        if(do_free) {
            pa = PTE2PA(*pte);
            kfree((void *)pa);
        }
        *pte = 0;
    }
}

// 减小用户进程的地址空间
// 如果旧的页面较小直接返回
// 大小不需要对齐页面
// 返回新的大小
uint64 uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz) {
    int npages;
    if(oldsz <= newsz) {
        return oldsz;
    }
    oldsz = PGROUNDUP(oldsz);
    newsz = PGROUNDUP(newsz);
    // 不同进程虚拟地址空间相互独立
    // 地址空间的地址范围是 0...sz
    if(oldsz > newsz) {
        npages = (oldsz - newsz) / PGSIZE;
        uvmunmap(pagetable, npages, newsz, 1);
    }
    return newsz;
}

// 增大用户进程的地址空间
// 如果旧的页面较大直接返回
// 大小不需要对齐页面
// 返回新的大小
// 返回零表示无内村或映射出现错误
uint64 uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm) {
    char *mem;
    if(oldsz >= newsz) {
        return oldsz;
    }
    oldsz = PGROUNDUP(oldsz);
    for(uint64 a = oldsz; a < newsz; a += PGSIZE) {
        // 内存不足需要释放已分配页
        mem = (char *)kalloc();
        if(mem == 0) {
            uvmdealloc(pagetable, a, oldsz);
            return 0;
        }
        memset(pagetable, 0, PGSIZE);
        // 如果映射失败需要删除映射项
        // 同时删除这一步分配的物理页
        if((mappages(pagetable, a, (uint64)mem, PGSIZE, PTE_R | PTE_U | xperm)) == -1) {
            uvmdealloc(pagetable, a, oldsz);
            kfree((void *)mem);
            return 0;
        }
    }
    return newsz;
}

// 清除内存页以及页表项
void uvmfree(pagetable_t pagetable, uint64 sz) {
    uvmunmap(pagetable, 0, PGROUNDUP(sz) / PGSIZE, 1);
    freewalk(pagetable);
}

// 拷贝父进程的页表和内存到子进程
// 成功返回 0，失败返回 -1
// 失败后返回已分配的内存
int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz) {
    pte_t *pte;
    uint64 pa;
    uint flags;
    char *mem;
    for(uint64 va = 0; va < sz; va += PGSIZE) {
        pte = walk(old, va, 0);
        if(pte == 0) {
            panic("uvmcopy: pte should exist");
        }
        if(*pte | PTE_V == 0) {
            panic("uvmcopy: page not present");
        }
        pa = PTE2PA(*pte);
        flags = PTE_FLAGS(*pte);
        mem = (char *)kalloc();
        if(mem == 0) {
            uvmunmap(new, va, PGSIZE, 0);
            return -1;
        }
        if(mappages(new, va, (uint64)mem, PGSIZE, flags) == -1) {
            uvmunmap(new, va, PGSIZE, 1);
            return -1;
        }
        memmove((void *)mem, (void *)pa, PGSIZE);
        
    }
    return 0;
}

// 标记一个页表项为用户不可见
// 用作用户栈保护页
void uvmclear(pagetable_t pagetable, uint64 va) {
    pte_t *pte = walk(pagetable, va, 0);
    if(pte == 0) {
        panic("uvmclear");
    }
    *pte &= ~PTE_V;
}

// 从内核空间拷贝数据到用户空间
// 成功返回 0，失败返回 -1
int copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len) {
    pte_t *pte;
    uint64 va0, n, offset, pa0;
    while(len > 0) {
        // 计算页面的初始位置及页内偏移量
        va0 = PGROUNDDOWN(dstva);
        offset = dstva - va0;

        if(va0 >= MAXVA) {
            return -1;
        }
        pte = walk(pagetable, va0, 0);
        if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_W) == 0 || 
            (*pte & PTE_U) == 0) {
            return -1;
        }
        pa0 = PTE2PA(*pte);


        // 计算当前页面需要复制多少字节
        n = PGSIZE - (dstva - va0);
        if(n > len) {
            n = len;
        }

        memmove((void *)(pa0 + offset), (void *)src, n);
        src += n;
        len -= n;
        dstva = va0 + PGSIZE;
    }
    return 0;
}

// 从用户空间拷贝数据到内核空间
// 成功返回 0，失败返回 -1
int copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len) {
    uint64 va0, offset, pa0, n;
    while(len > 0) {
        // 找到页面起始位置及偏移量
        va0 = PGROUNDDOWN(srcva);
        if(va0 >= MAXVA) {
            return -1;
        }
        offset = srcva - va0;

        pa0 = walkaddr(pagetable, va0);
        if(pa0 == 0) {
            return -1;
        }

        // 计算当前页面需要复制多少字节
        n = PGSIZE - (srcva - va0);
        if(n > len) {
            n = len;
        }

        memmove((void *)dst, (void *)(pa0 + offset), n);
        dst += n;
        srcva += n;
        len -= n;
        
    }
    return 0;
}

// 从用户空间拷贝字符串到内核空间
// 直到遇到零字符或达到最大值
// 遇到零字符结束则返回 0
int copyinstr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max) {
    uint64 va0, pa0, offset, n;
    char *p;
    int is_null = 0;
    while(max > 0 && is_null == 0) {
        va0 = PGROUNDDOWN(srcva);
        if(va0 >= MAXVA) {
            return -1;
        }
        pa0 = walkaddr(pagetable, va0);
        if(pa0 == 0) {
            return -1;
        }
        n = PGSIZE - (srcva - va0);
        if(n > max) {
            n = max;
        }
        while(n > 0) {
            offset = srcva - va0;
            p = (char *)(srcva + offset);
            *dst = *p;
            if(*p == '\0') {
                is_null = 1;
                break;
            }
            dst++;
            srcva++;
            n--;
            max--;
        }
    }
    if(is_null == 0) {
        return -1;
    }
    return 0;
}


