// 避免编译器错误解析
#ifndef __ASSEMBLER__

// CPU ID
static inline uint64 r_mhartid(void)
{
	uint64 x;
	asm volatile("csrr %0, mhartid" : "=r" (x));
	return x;
}

// Machine Status Register

#define MSTATUS_MPP_MASK (3L << 11)
#define MSTATUS_MPP_M (3L << 11)
#define MSTATUS_MPP_S (1L << 11)
#define MSTATUS_MPP_U (0L << 11)
#define MSTATUS_MIE (1L << 3) // 启用中断

static inline uint64 r_mstatus(void) 
{
	uint64 x;
    asm volatile("csrr %0, mstatus" : "=r" (x));
    return x;
}

static inline void w_mstatus(uint64 x) 
{
	asm volatile("csrw mstatus, %0" : : "r" (x));
}

// Supervisor Status Register

static inline uint64 r_sstatus(void)
{
    uint64 x;
    asm volatile("csrr %0, sstatus" : "=r" (x));
    return x;
}

static inline void w_sstatus(uint64 x)
{
    asm volatile("csrw sstatus, %0" : : "r" (x));
}

// Machine  Exception Program Counter

static inline uint64 r_mepc(void)
{
    uint64 x;
    asm volatile("csrr %0, mepc" : "=r" (x));
    return x;
}

static inline void w_mepc(uint64 x)
{
    asm volatile("csrw mepc, %0" : : "r" (x));
}


// Supervisor Exception Program Counter

static inline uint64 r_sepc(void)
{
    uint64 x;
    asm volatile("csrr %0, sepc" : "=r" (x));
    return x;
}

static inline void w_sepc(uint64 x)
{
    asm volatile("csrw sepc, %0" : : "r" (x));
}

// Supervisor Address Translation and Protection

static inline uint64 r_satp(void)
{
    uint64 x;
    asm volatile("csrr %0, satp" : "=r" (x));
    return x;
}

static inline void w_satp(uint64 x)
{
    asm volatile("csrw satp, %0" : : "r" (x));
}

// Machine Exception Delegation
static inline uint64 r_medeleg(void)
{
    uint64 x;
    asm volatile("csrr %0, medeleg" : "=r" (x));
    return x;
}

static inline void w_medeleg(uint64 x)
{
    asm volatile("csrw medeleg, %0" : : "r" (x));
}

// Machine Interrupt Delegation

static inline uint64 r_mideleg(void)
{
    uint64 x;
    asm volatile("csrr %0, mideleg" : "=r" (x));
    return x;
}

static inline void w_mideleg(uint64 x)
{
    asm volatile("csrw mideleg, %0" : : "r" (x));
}

// Supervisor Interrupt Enable

#define SIE_SEIE (1L << 9) // external
#define SIE_STIE (1L << 5) // timer
#define SIE_SSIE (1L << 1) // software

static inline uint64 r_sie(void)
{
    uint64 x;
    asm volatile("csrr %0, sie" : "=r" (x));
    return x;
}

static inline void w_sie(uint64 x)
{
    asm volatile("csrw sie, %0" : : "r" (x));
}

// Physical Memory Protection

static inline void w_pmpcfg0(uint64 x)
{
  asm volatile("csrw pmpcfg0, %0" : : "r" (x));
}

static inline void w_pmpaddr0(uint64 x)
{
  asm volatile("csrw pmpaddr0, %0" : : "r" (x));
}

// Machine-mode Interrupt Enable

#define MIE_MEIE (1L << 11) // external
#define MIE_MTIE (1L << 7)  // timer
#define MIE_MSIE (1L << 3)  // software

static inline uint64 r_mie()
{
  uint64 x;
  asm volatile("csrr %0, mie" : "=r" (x) );
  return x;
}

static inline void w_mie(uint64 x)
{
  asm volatile("csrw mie, %0" : : "r" (x));
}

// Machine-mode Interrupt Vector
static inline void w_mtvec(uint64 x)
{
  asm volatile("csrw mtvec, %0" : : "r" (x));
}

// Thread Pointer

static inline uint64 r_tp()
{
  uint64 x;
  asm volatile("mv %0, tp" : "=r" (x));
  return x;
}

static inline void w_tp(uint64 x)
{
  asm volatile("mv tp, %0" : : "r" (x));
}

// Machine-mode Scratch
static inline void w_mscratch(uint64 x)
{
  asm volatile("csrw mscratch, %0" : : "r" (x));
}

#endif // __ASSEMBLER__
