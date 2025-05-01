#include "riscv.h"

pagetable_t kernel_pagetable;
extern char etext[];        // from kernel.ld
extern char trampoline[];   // from trampoline.S


