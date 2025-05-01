// risc-v Sv39 模式有三级页表
// 单个页表项页面包含512个64位的 PTEs
// 单个64位虚拟地址被分成5个区域：
// 39...63 -- 必须为零
// 30...38 -- 9位二级索引
// 21...29 -- 9位一级索引
// 12...20 -- 9位零级索引
// 00...11 -- 12位页内偏移

#include "riscv.h"

pagetable_t kernel_pagetable;
extern char etext[];        // from kernel.ld
extern char trampoline[];   // from trampoline.S

// 返回虚拟地址对应的物理地址
// 创建需要的页表项
pte_t *walk(pagetable_t pagetable, uint64 va, int alloc) {
    if(va >= MAXVA) {
        panic("walk");
    }
    for()
}