#include "param.h"
#include "riscv.h"

// 物理内存布局

// According to qemu's hw/riscv/virt.c:
// 00001000 -- qemu's boot ROM
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0
// 10001000 -- virtio disk
// 80000000 -- boot jumps hear
// unused RAM after 80000000.

// 通用异步收发传输器
#define UART0 0x10000000L
#define UART0_IRQ 10

// 虚拟化I/O设备
#define VIRTIO0 0x10001000L
#define VIRTIO0_IRQ 1

// 核心本地中断控制器
#define CLINT 0x2000000L
#define CLINT_MTIMECMP(hartid) (CLINT + 0x4000 + 8*(hartid))
#define CLINT_MTIME (CLINT + 0xBFF8) // cycles since boot.

// 平台级中断控制器
#define PLIC 0x0c000000L
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + (hart)*0x100)
#define PLIC_SENABLE(hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + (hart)*0x2000)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000)

// RAM memory:
// text
// data and bss
// fixed-size stack
// expandable heap
// ...
// KSTACK
// TRAPFRAME
// TRAMPOLINE

// RAM 物理地址范围
#define KERNBASE = 0x80000000L;
#define PHYSTOP (KERNBASE + MEMSIZE)

// 从地址最高位起，
// 由高至低依次映射：
// 绷床页、陷阱帧、内核栈
#define TRAMPOLINE (MAXVA - PGSIZE)
#define TRAPFRAME (TRAMPOLINE - PGSIZE)
#define KSTACK(p) (TRAMPOLINE - (p + 1) * 2 * PGSIZE)
