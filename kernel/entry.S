# 设备树中显示内存 RAM 基地址为 0x80000000
# qemu -kernel 让每个 CPU 跳转到地址 0x80000000
# kernel.ld 把以下代码放到地址 0x80000000

.section .text
.global _entry
_entry:
    # 把栈帧 sp 设置为 (CPUID+1) * 4096
    la sp, stack0
    li a0, 4096
    csrr a1, mhartid
    addi a1, a1, 1
    mul a0, a0, a1
    add sp, sp, a0
    # 跳转到 start 函数
    call start

spin:
    j spin

