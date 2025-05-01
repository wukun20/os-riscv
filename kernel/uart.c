//
// 16550a UART 低级驱动例程
//

#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "types.h"

extern volatile int panicked; // from printf.c

// UART 控制寄存器已经映射到内存 UART0 
#define Reg(reg) ((volatile unsigned char *)(UART0 + (reg)))

// UART 控制寄存器，参考 http://byterunner.com/16550.html
#define RHR 0                   // 接受保持寄存器（输入字节）
#define THR 0                   // 发送保持寄存器（输出字节）
#define IER 1                   // 中断使能寄存器
#define IER_RX_ENABLE (1<<0)    // 接受中断使能
#define IER_TX_ENABLE (1<<1)    // 发送中断使能
#define FCR 2                   // FIFO 控制寄存器
#define FCR_FIFO_ENABLE (1<<0)  // FIFO 使能寄存器
#define FCR_FIFO_CLEAR (3<<1)   // 清空两个 FIFOs
#define ISR 2                   // 中断状态寄存器
#define LCR 3                   // 行控制寄存器
#define LCR_EIGHT_BITS (3<<0)   // 数据位为 8
#define LCR_BAUD_LATCH (1<<7)   // 设置比特率模式
#define LSR 5                   // 行状态寄存器
#define LSR_RX_READY (1<<0)     // RHR 输入等待被读取
#define LSR_TX_IDLE (1<<5)      // THR 可接受发送数据

#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg, v) (*(Reg(reg)) = (v))

#define UART_TX_BUF_SIZE 32

static struct {
    struct spinlock lock;
    char buf[UART_TX_BUF_SIZE];
    uint64 read;    // 下一个读出缓存的位置
    uint64 write;   // LSR_TX_IDLE下一个写入缓存的位置
} uarttx;

void uartinit(void) {
    // 禁用中断
    WriteReg(IER, 0x00);

    // 进入设置波特率模式
    WriteReg(LCR, LCR_BAUD_LATCH);

    // 设置波特率为 38.4K.
    WriteReg(0, 0x03);
    WriteReg(1, 0x00);

    // 设置字长为 8 位
    WriteReg(LCR, LCR_EIGHT_BITS);

    // 启用并清空 FIFOs.
    WriteReg(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);

    // 启用发送与接收中断
    WriteReg(IER, IER_TX_ENABLE | IER_RX_ENABLE);

    initlock(&uarttx.lock, "uarttx");
}

// 
static void uartstart(void) {
    while(1) {
        // 传输缓冲区为空，读取重置中断标志位
        if(uarttx.read == uarttx.write) {
            ReadReg(ISR);
            return;
        }
        // 发送保持寄存器被占用，直接返回
        if((ReadReg(LSR) & LSR_TX_IDLE) == 0) {
            return;
        }
        int c = uarttx.buf[uarttx.read++];
        uarttx.read %= UART_TX_BUF_SIZE;
        // uartputc() 可能在等待缓存空位
        wakeup(&uarttx.write);
        WriteReg(THR, c);
    }
}

void uartputc(int c) {
    acquire(&uarttx.lock);
    // 自选锁发生重入则内核恐慌
    if(panicked) {
        for(;;) {
        }
    }
    // 缓冲区满则进入睡眠
    while(uarttx.read == uarttx.write) {
        sleep(&uarttx.write, &uarttx.lock);
    }
    // 写入缓冲区并传到设备
    uarttx.buf[uarttx.write++] = c;
    uarttx.write % UART_TX_BUF_SIZE;
    uartstart();
    release(&uarttx.lock);
}

void uartputc_sync(int c) {
    push_off();
    // 若内核恐慌则进入循环，关中断下会冻结系统
    if(panicked) {
        for(;;) { 
        }
    }
    while((ReadReg(LSR) & LSR_TX_IDLE) == 0) {
    }
    WriteReg(THR, c);
    pop_off();
}

// 返回读取到的字符，若无返回 -1
int uartgetc(void) {
    if(ReadReg(LSR) & LSR_RX_READY) {
        return ReadReg(RHR);
    } else {
        return -1;
    }
}

// 处理设备中断
void uartintr(void) {
    int c;
    // 读取单个字符
    while(1) {
        c = uartgetc();
        if(c == -1) {
            break;
        }
        consoleintr(c);
    }
    // 发送缓冲区数据
    acquire(&uarttx.lock);
    uartstart();
    release(&uarttx.lock);
}
