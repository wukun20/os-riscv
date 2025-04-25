
kernel/kernel:     file format elf64-littleriscv


Disassembly of section .text:

0000000080000000 <_entry>:
    80000000:	00001117          	auipc	sp,0x1
    80000004:	03813103          	ld	sp,56(sp) # 80001038 <_GLOBAL_OFFSET_TABLE_+0x8>
    80000008:	6505                	lui	a0,0x1
    8000000a:	f14025f3          	csrr	a1,mhartid
    8000000e:	0585                	addi	a1,a1,1
    80000010:	02b50533          	mul	a0,a0,a1
    80000014:	912a                	add	sp,sp,a0
    80000016:	076000ef          	jal	ra,8000008c <start>

000000008000001a <spin>:
    8000001a:	a001                	j	8000001a <spin>

000000008000001c <timerinit>:
	// 跳转到 main 函数模式下的main
	asm volatile("mret");
}

void timerinit(void) 
{
    8000001c:	1141                	addi	sp,sp,-16
    8000001e:	e422                	sd	s0,8(sp)
    80000020:	0800                	addi	s0,sp,16

// CPU ID
static inline uint64 r_mhartid(void)
{
	uint64 x;
	asm volatile("csrr %0, mhartid" : "=r" (x));
    80000022:	f14027f3          	csrr	a5,mhartid
	// 获得处理器编号
	int id = r_mhartid();
    80000026:	0007859b          	sext.w	a1,a5

	// 设置中断间隔为1000000个时钟周期（about 0.1s in qemu）
	// 设置下一个时钟中断时刻
	int interval = 1000000;
	*(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval;
    8000002a:	0037979b          	slliw	a5,a5,0x3
    8000002e:	02004737          	lui	a4,0x2004
    80000032:	97ba                	add	a5,a5,a4
    80000034:	0200c737          	lui	a4,0x200c
    80000038:	ff873703          	ld	a4,-8(a4) # 200bff8 <_entry-0x7dff4008>
    8000003c:	000f4637          	lui	a2,0xf4
    80000040:	24060613          	addi	a2,a2,576 # f4240 <_entry-0x7ff0bdc0>
    80000044:	9732                	add	a4,a4,a2
    80000046:	e398                	sd	a4,0(a5)

	// 暂存中断时间和间隔
	uint64 *scratch = &timer_scratch[id][0];
    80000048:	00259693          	slli	a3,a1,0x2
    8000004c:	96ae                	add	a3,a3,a1
    8000004e:	068e                	slli	a3,a3,0x3
    80000050:	00001717          	auipc	a4,0x1
    80000054:	fb070713          	addi	a4,a4,-80 # 80001000 <timer_scratch>
    80000058:	9736                	add	a4,a4,a3
	scratch[3] = CLINT_MTIMECMP(id);
    8000005a:	ef1c                	sd	a5,24(a4)
	scratch[4] = interval;
    8000005c:	f310                	sd	a2,32(a4)
}

// Machine-mode Scratch
static inline void w_mscratch(uint64 x)
{
  asm volatile("csrw mscratch, %0" : : "r" (x));
    8000005e:	34071073          	csrw	mscratch,a4
  asm volatile("csrw mtvec, %0" : : "r" (x));
    80000062:	00001797          	auipc	a5,0x1
    80000066:	fe67b783          	ld	a5,-26(a5) # 80001048 <_GLOBAL_OFFSET_TABLE_+0x18>
    8000006a:	30579073          	csrw	mtvec,a5
    asm volatile("csrr %0, mstatus" : "=r" (x));
    8000006e:	300027f3          	csrr	a5,mstatus
	w_mscratch((uint64)scratch);

	// 启用机器模式下时钟中断
	w_mtvec((uint64)timervec);
	w_mstatus(r_mstatus() | MSTATUS_MIE);
    80000072:	0087e793          	ori	a5,a5,8
	asm volatile("csrw mstatus, %0" : : "r" (x));
    80000076:	30079073          	csrw	mstatus,a5
  asm volatile("csrr %0, mie" : "=r" (x) );
    8000007a:	304027f3          	csrr	a5,mie
	w_mie(r_mie() | MIE_MTIE);
    8000007e:	0807e793          	ori	a5,a5,128
  asm volatile("csrw mie, %0" : : "r" (x));
    80000082:	30479073          	csrw	mie,a5
}
    80000086:	6422                	ld	s0,8(sp)
    80000088:	0141                	addi	sp,sp,16
    8000008a:	8082                	ret

000000008000008c <start>:
{
    8000008c:	1141                	addi	sp,sp,-16
    8000008e:	e406                	sd	ra,8(sp)
    80000090:	e022                	sd	s0,0(sp)
    80000092:	0800                	addi	s0,sp,16
    asm volatile("csrr %0, mstatus" : "=r" (x));
    80000094:	300027f3          	csrr	a5,mstatus
	x &= ~MSTATUS_MPP_MASK;
    80000098:	7779                	lui	a4,0xffffe
    8000009a:	7ff70713          	addi	a4,a4,2047 # ffffffffffffe7ff <stack0+0xffffffff7fffd79f>
    8000009e:	8ff9                	and	a5,a5,a4
	x |= MSTATUS_MPP_S;
    800000a0:	6705                	lui	a4,0x1
    800000a2:	80070713          	addi	a4,a4,-2048 # 800 <_entry-0x7ffff800>
    800000a6:	8fd9                	or	a5,a5,a4
	asm volatile("csrw mstatus, %0" : : "r" (x));
    800000a8:	30079073          	csrw	mstatus,a5
    asm volatile("csrw mepc, %0" : : "r" (x));
    800000ac:	00001797          	auipc	a5,0x1
    800000b0:	f947b783          	ld	a5,-108(a5) # 80001040 <_GLOBAL_OFFSET_TABLE_+0x10>
    800000b4:	34179073          	csrw	mepc,a5
    asm volatile("csrw satp, %0" : : "r" (x));
    800000b8:	4781                	li	a5,0
    800000ba:	18079073          	csrw	satp,a5
    asm volatile("csrw medeleg, %0" : : "r" (x));
    800000be:	67c1                	lui	a5,0x10
    800000c0:	17fd                	addi	a5,a5,-1 # ffff <_entry-0x7fff0001>
    800000c2:	30279073          	csrw	medeleg,a5
    asm volatile("csrw mideleg, %0" : : "r" (x));
    800000c6:	30379073          	csrw	mideleg,a5
    asm volatile("csrr %0, sie" : "=r" (x));
    800000ca:	104027f3          	csrr	a5,sie
	w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
    800000ce:	2227e793          	ori	a5,a5,546
    asm volatile("csrw sie, %0" : : "r" (x));
    800000d2:	10479073          	csrw	sie,a5
  asm volatile("csrw pmpaddr0, %0" : : "r" (x));
    800000d6:	57fd                	li	a5,-1
    800000d8:	83a9                	srli	a5,a5,0xa
    800000da:	3b079073          	csrw	pmpaddr0,a5
  asm volatile("csrw pmpcfg0, %0" : : "r" (x));
    800000de:	47bd                	li	a5,15
    800000e0:	3a079073          	csrw	pmpcfg0,a5
	timerinit();
    800000e4:	f39ff0ef          	jal	ra,8000001c <timerinit>
	asm volatile("csrr %0, mhartid" : "=r" (x));
    800000e8:	f14027f3          	csrr	a5,mhartid
  asm volatile("mv tp, %0" : : "r" (x));
    800000ec:	823e                	mv	tp,a5
	asm volatile("mret");
    800000ee:	30200073          	mret
}
    800000f2:	60a2                	ld	ra,8(sp)
    800000f4:	6402                	ld	s0,0(sp)
    800000f6:	0141                	addi	sp,sp,16
    800000f8:	8082                	ret
    800000fa:	0000                	unimp
    800000fc:	0000                	unimp
	...

0000000080000100 <kernelvec>:
    80000100:	7111                	addi	sp,sp,-256
    80000102:	e006                	sd	ra,0(sp)
    80000104:	e40a                	sd	sp,8(sp)
    80000106:	e80e                	sd	gp,16(sp)
    80000108:	ec12                	sd	tp,24(sp)
    8000010a:	f016                	sd	t0,32(sp)
    8000010c:	f41a                	sd	t1,40(sp)
    8000010e:	f81e                	sd	t2,48(sp)
    80000110:	fc22                	sd	s0,56(sp)
    80000112:	e0a6                	sd	s1,64(sp)
    80000114:	e4aa                	sd	a0,72(sp)
    80000116:	e8ae                	sd	a1,80(sp)
    80000118:	ecb2                	sd	a2,88(sp)
    8000011a:	f0b6                	sd	a3,96(sp)
    8000011c:	f4ba                	sd	a4,104(sp)
    8000011e:	f8be                	sd	a5,112(sp)
    80000120:	fcc2                	sd	a6,120(sp)
    80000122:	e146                	sd	a7,128(sp)
    80000124:	e54a                	sd	s2,136(sp)
    80000126:	e94e                	sd	s3,144(sp)
    80000128:	ed52                	sd	s4,152(sp)
    8000012a:	f156                	sd	s5,160(sp)
    8000012c:	f55a                	sd	s6,168(sp)
    8000012e:	f95e                	sd	s7,176(sp)
    80000130:	fd62                	sd	s8,184(sp)
    80000132:	e1e6                	sd	s9,192(sp)
    80000134:	e5ea                	sd	s10,200(sp)
    80000136:	e9ee                	sd	s11,208(sp)
    80000138:	edf2                	sd	t3,216(sp)
    8000013a:	f1f6                	sd	t4,224(sp)
    8000013c:	f5fa                	sd	t5,232(sp)
    8000013e:	f9fe                	sd	t6,240(sp)
    80000140:	07a000ef          	jal	ra,800001ba <kerneltrap>
    80000144:	6082                	ld	ra,0(sp)
    80000146:	6122                	ld	sp,8(sp)
    80000148:	61c2                	ld	gp,16(sp)
    8000014a:	7282                	ld	t0,32(sp)
    8000014c:	7322                	ld	t1,40(sp)
    8000014e:	73c2                	ld	t2,48(sp)
    80000150:	7462                	ld	s0,56(sp)
    80000152:	6486                	ld	s1,64(sp)
    80000154:	6526                	ld	a0,72(sp)
    80000156:	65c6                	ld	a1,80(sp)
    80000158:	6666                	ld	a2,88(sp)
    8000015a:	7686                	ld	a3,96(sp)
    8000015c:	7726                	ld	a4,104(sp)
    8000015e:	77c6                	ld	a5,112(sp)
    80000160:	7866                	ld	a6,120(sp)
    80000162:	688a                	ld	a7,128(sp)
    80000164:	692a                	ld	s2,136(sp)
    80000166:	69ca                	ld	s3,144(sp)
    80000168:	6a6a                	ld	s4,152(sp)
    8000016a:	7a8a                	ld	s5,160(sp)
    8000016c:	7b2a                	ld	s6,168(sp)
    8000016e:	7bca                	ld	s7,176(sp)
    80000170:	7c6a                	ld	s8,184(sp)
    80000172:	6c8e                	ld	s9,192(sp)
    80000174:	6d2e                	ld	s10,200(sp)
    80000176:	6dce                	ld	s11,208(sp)
    80000178:	6e6e                	ld	t3,216(sp)
    8000017a:	7e8e                	ld	t4,224(sp)
    8000017c:	7f2e                	ld	t5,232(sp)
    8000017e:	7fce                	ld	t6,240(sp)
    80000180:	6111                	addi	sp,sp,256
    80000182:	10200073          	sret
    80000186:	00000013          	nop
    8000018a:	00000013          	nop
    8000018e:	0001                	nop

0000000080000190 <timervec>:
    80000190:	34051573          	csrrw	a0,mscratch,a0
    80000194:	e10c                	sd	a1,0(a0)
    80000196:	e510                	sd	a2,8(a0)
    80000198:	e914                	sd	a3,16(a0)
    8000019a:	6d0c                	ld	a1,24(a0)
    8000019c:	7110                	ld	a2,32(a0)
    8000019e:	6194                	ld	a3,0(a1)
    800001a0:	96b2                	add	a3,a3,a2
    800001a2:	e194                	sd	a3,0(a1)
    800001a4:	4589                	li	a1,2
    800001a6:	14459073          	csrw	sip,a1
    800001aa:	6914                	ld	a3,16(a0)
    800001ac:	6510                	ld	a2,8(a0)
    800001ae:	610c                	ld	a1,0(a0)
    800001b0:	34051573          	csrrw	a0,mscratch,a0
    800001b4:	0000                	unimp
    800001b6:	0000                	unimp
	...

00000000800001ba <kerneltrap>:
void kerneltrap(void) 
{
    800001ba:	1141                	addi	sp,sp,-16
    800001bc:	e422                	sd	s0,8(sp)
    800001be:	0800                	addi	s0,sp,16
	while(1);
    800001c0:	a001                	j	800001c0 <kerneltrap+0x6>

00000000800001c2 <main>:
volatile static int started = 0;

void main(void) 
{
    800001c2:	1141                	addi	sp,sp,-16
    800001c4:	e422                	sd	s0,8(sp)
    800001c6:	0800                	addi	s0,sp,16
	if(started == 0) {
    800001c8:	00001797          	auipc	a5,0x1
    800001cc:	e607a783          	lw	a5,-416(a5) # 80001028 <started>
    800001d0:	eb89                	bnez	a5,800001e2 <main+0x20>
		started = 1;
    800001d2:	4785                	li	a5,1
    800001d4:	00001717          	auipc	a4,0x1
    800001d8:	e4f72a23          	sw	a5,-428(a4) # 80001028 <started>
	} else {
		started = 0;
	}
}
    800001dc:	6422                	ld	s0,8(sp)
    800001de:	0141                	addi	sp,sp,16
    800001e0:	8082                	ret
		started = 0;
    800001e2:	00001797          	auipc	a5,0x1
    800001e6:	e407a323          	sw	zero,-442(a5) # 80001028 <started>
}
    800001ea:	bfcd                	j	800001dc <main+0x1a>
	...
