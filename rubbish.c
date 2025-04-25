// entry.S 需要为每个 CPU 分配栈帧
__attribute__ ((aligned (16))) char stack0[4096 * 1];

// 按照设备树信息设置基地址
#define UART0 0x10000000L
#define KERNBASE 0x80000000L

// UART 寄存器映射到了内存地址 UART0 中
// 通过偏移量读写不同寄存器
// 参考 http://byterunner.com/16550.html
#define Reg(reg) ((volatile unsigned char *)(UART0 + reg))

#define RHR 0                   // 接受保持寄存器(用于输入字节)
#define THR 0                 	// 发送保持寄存器(用于输出字节)
#define IER 1                 	// 中断使能寄存器
#define IER_RX_ENABLE (1<<0)	// 启动接受中断的位掩码
#define IER_TX_ENABLE (1<<1)	// 启动发送中断的位掩码
#define FCR 2                 	// 缓冲区控制寄存器
#define FCR_FIFO_ENABLE (1<<0)	// 启动缓冲区的位掩码
#define FCR_FIFO_CLEAR (3<<1) 	// 清空缓冲区的位掩码
#define ISR 2                 	// 中断状态寄存器
#define LCR 3                 	// 行控制寄存器
#define LCR_EIGHT_BITS (3<<0) 	// 行数据位数位掩码
#define LCR_BAUD_LATCH (1<<7) 	// 波特率设置寄存器
#define LSR 5                 	// 行状态寄存器
#define LSR_RX_READY (1<<0)   	// 行数据就绪位掩码
#define LSR_TX_IDLE (1<<5)    	// 行数据空闲位掩码

#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg, v) (*(Reg(reg)) = v)

void uartinit(void) 
{
    // 禁用中断
    WriteReg(IER, 0x00);
    
    // 设置 38.4K 波特率
    WriteReg(LCR, LCR_BAUD_LATCH);
    WriteReg(0,0x03);
    WriteReg(1,0x00);
    
    // 设置数据宽度为 8 比特
    WriteReg(LCR, LCR_EIGHT_BITS);
    
    // 启用并初始化缓冲区
    WriteReg(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);
    
    // 启用发送与接受中断
    WriteReg(IER, IER_TX_ENABLE | IER_RX_ENABLE);
}

void uartputc_sync(int c)
{
    while((ReadReg(LSR) & LSR_TX_IDLE) == 0) 
        ;
    WriteReg(THR, c);
}

int strlen(const char *s)
{
    int n;
    for(n = 0; s[n]; n++)
        ;
    return n;
}

void start(void)
{
    const char *str = "hello";
    
}
