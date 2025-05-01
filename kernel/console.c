// 控制与外设交互
#include "defs.h"
#include "file.h"
#include "spinlock.h"

#define INPUT_BUF_SIZE 128

static struct {
    struct spinlock lock;
    char buf[INPUT_BUF_SIZE];
    uint r, w, e; // 读取、写入、编辑
} cons;

// 复制一行输入数据至 dst
// user_dst 表明数据来自用户或内核地址
int consoleread(int user_dst, uint64 dst, int n) {
    
}

int consolewrite(int user_src, uint64 src, int n) {
    
}

void consoleinit(void) {
    initlock(&cons.lock, "cons");
    uartinit();
    // 连接文件与设备系统
    devsws[CONSOLE].read = consoleread;
    devsws[CONSOLE].write = consolewrite;
}

void consputc(int c) {

}

// 处理控制台输入中断
void consoleintr(int c) {

}