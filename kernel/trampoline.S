#
# 链接脚本将该代码的起始位置限制到一个页面边界
# 该页面被映射到用户和内核空间中的相同虚拟地址
# 

# trap.c 设置 stvec 跳转到此处
# 从用户态转换到内核态
.section trampsec
.globl trampoline
trampoline:
.align 4
.globl uservec
uservec:
    // 保存 a0 到 sscratch
    scrw sscratch, a0

    // 每个进程都有独立的 p->trapframe 内存区域
    // 将用户寄存器保存至 TRAPFRAME
    li a0, TRAPFRAME
    sd ra, 40(a0)
    sd sp, 48(a0)
    sd gp, 56(a0)
    sd tp, 64(a0)
    sd t0, 72(a0)
    sd t1, 80(a0)
    sd t2, 88(a0)
    sd s0, 96(a0)
    sd s1, 104(a0)
    sd a1, 120(a0)
    sd a2, 128(a0)
    sd a3, 136(a0)
    sd a4, 144(a0)
    sd a5, 152(a0)
    sd a6, 160(a0)
    sd a7, 168(a0)
    sd s2, 176(a0)
    sd s3, 184(a0)
    sd s4, 192(a0)
    sd s5, 200(a0)
    sd s6, 208(a0)
    sd s7, 216(a0)
    sd s8, 224(a0)
    sd s9, 232(a0)
    sd s10, 240(a0)
    sd s11, 248(a0)
    sd t3, 256(a0)
    sd t4, 264(a0)
    sd t5, 272(a0)
    sd t6, 280(a0)
    # 借助寄存器 t0 保存 sscratch
    scrr t0, sscratch
    sd t0, 112(a0)

    # 取出内核页表至 t1
    ld t1, 0(a0)
    # 取出栈顶指针至 sp
    ld sp, 8(a0)
    # 取出 usertrap() 地址至 t0
    ld t0, 16(a0)
    # 取出当前处理器编号至 tp
    ld tp, 32(a0)

    # 确保所有的内存操作完成
    sfence.vma zero, zero
    # 载入内核页表
    csrw satp, t1
    # 刷新已过时的用户页表条目
    sfence.vma zero, zero

    # 跳转至 trap.c 中的 usertrap() 
    jr t0;


# usertrapret() in trap.c 跳转到此
# 从内核态转换到用户态
.globl userret
userret:
    # 取出 a0 至 satp
    sfence.vma zero, zero
    csrw satp, a0
    sfence.vma zero, zero

    li a0, TRAPFRAME
    ld ra, 40(a0)
    ld sp, 48(a0)
    ld gp, 56(a0)
    ld tp, 64(a0)
    ld t0, 72(a0)
    ld t1, 80(a0)
    ld t2, 88(a0)
    ld s0, 96(a0)
    ld s1, 104(a0)
    ld a0, 112(a0)
    ld a1, 120(a0)
    ld a2, 128(a0)
    ld a3, 136(a0)
    ld a4, 144(a0)
    ld a5, 152(a0)
    ld a6, 160(a0)
    ld a7, 168(a0)
    ld s2, 176(a0)
    ld s3, 184(a0)
    ld s4, 192(a0)
    ld s5, 200(a0)
    ld s6, 208(a0)
    ld s7, 216(a0)
    ld s8, 224(a0)
    ld s9, 232(a0)
    ld s10, 240(a0)
    ld s11, 248(a0)
    ld t3, 256(a0)
    ld t4, 264(a0)
    ld t5, 272(a0)
    ld t6, 280(a0)

    # usertrapret() 设置 sstatus, sepc
    # 返回至用户态及用户程序寄存器
    sret
