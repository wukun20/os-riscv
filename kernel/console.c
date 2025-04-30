// 控制与外设交互
#include "defs.h"
#include "spinlock.h"

#define INPUT_BUF_SIZE 128

static struct {
    struct spinlock lock;
    char buf[INPUT_BUF_SIZE];
    uint r, w, e; // 读取、写入、编辑
} cons;

int consoleread(int user_dst, uint64 dst, int n) {
    
}

void consoleinit(void) {
    initlock(&cons.lock, "cons");
    uartinit();

}