#include "fs.h"
#include "sleeplock.h"
#include "types.h"

#define major(dev) ((dev) >> 16 & 0xFFFF)
#define minor(dev)  ((dev) & 0xFFFF)
#define	mkdev(m,n)  ((uint)((m) << 16 | (n)))

#define CONSOLE 1

struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type;
    int ref;            // 引用计数
    char readable;
    char writable;
    struct pipe *pipe;  // FD_PIPE
    struct inode *ino;  // FD_INODE and FD_DEVICE
    uint off;           // FD_INODE
    short major;        // FD_DEVICE
};

struct inode {
    uint dev;               // 设备编号
    uint inum;              // 节点编号
    int ref;                // 引用计数
    struct sleeplock lock;  // 保护以下内容
    int valid;              // 节点是否从磁盘中读取

    short type;             // 复制磁盘节点
    short major;
    short minor;
    short nlink;
    uint size;
    uint addrs[NDIRECT + 1];
};

struct devsw {
    int (*read)(int, uint64, int);
    int (*write)(int, uint64, int);
};


