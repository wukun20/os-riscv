#include "types.h"
#include "param.h"
#include "riscv.h"
#include "memlayout.h"

extern void main();

void timerinit(void);
extern void timervec(void);
uint64 timer_scratch[NCPU][5]; // 计时器中断暂存状态

void start(void) 
{
	// 设置既往模式为监督模式
	uint64 x = r_mstatus();
	x &= ~MSTATUS_MPP_MASK;
	x |= MSTATUS_MPP_S;
	w_mstatus(x);

	// 设置异常程序为main
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
	w_tp(r_mhartid());

	// 转换到跳转到监督者
	// 跳转到 main 函数模式下的main
	asm volatile("mret");
}

void timerinit(void) 
{
	// 获得处理器编号
	int id = r_mhartid();

	// 设置中断间隔为1000000个时钟周期（about 0.1s in qemu）
	// 设置下一个时钟中断时刻
	int interval = 1000000;
	*(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval;

	// 暂存中断时间和间隔
	uint64 *scratch = &timer_scratch[id][0];
	scratch[3] = CLINT_MTIMECMP(id);
	scratch[4] = interval;
	w_mscratch((uint64)scratch);

	// 启用机器模式下时钟中断
	w_mtvec((uint64)timervec);
	w_mstatus(r_mstatus() | MSTATUS_MIE);
	w_mie(r_mie() | MIE_MTIE);
}
