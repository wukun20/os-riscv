#include "types.h"
#include "param.h"
#include "riscv.h"
#include "memlayout.h"
#include "defs.h"

// entry.S 为每个处理器分配内核栈空间
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// 为每个处理器生成时钟中断
void timerinit(void) {
	// 启用监督者模式的时钟中断
	w_mie(r_mie() | MIE_STIE);

	// 开启 SSTC 拓展（如时钟比较功能）
	w_menvcfg(r_menvcfg() | (1L << 63));

	// 允许监督者模式使用时钟比较
	w_mcounteren(r_mcounteren() | 2);

	// 请求第一次时钟中断
	w_stimecmp(r_time() + 1000000);
}

// entry.S 在机器模式下跳转到此处
void start(void) {
	// 设置既往模式为监督模式
	uint64 x = r_mstatus();
	x &= ~MSTATUS_MPP_MASK;
	x |= MSTATUS_MPP_S;
	w_mstatus(x);

	// 设置异常程序为 main
	// 需要 gcc -mcmodel=medany
	w_mepc((uint64)main);

	// 禁止分页
	w_satp(0);

	// 委托所有中断和异常至监督模式
	// 开启外部、定时器、软件中断
	w_medeleg(0xffff);
	w_mideleg(0xffff);
	w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

	// 所有物理地址对监督模式完全可见
	w_pmpaddr0(0x3fffffffffffffull);
	w_pmpcfg0(0xf);
	
	// 初始化时钟中断
	timerinit();

	// 把 CPU'id 存入 tp 寄存器
	int id = r_mhartid();
	w_tp(id);

	// 转换到跳转到监督者
	// 跳转到 main 函数模式下的main
	asm volatile("mret");
}