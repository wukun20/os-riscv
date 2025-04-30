//
// 磁盘的内存副本
// [ boot block | super block | log | inode blocks | free bit map | data blocks ]
//
#include "types.h"

#define BOOTINO 1   // 根索引号
#define BSIZE 1024  // 块大小

#define FSMAGIC 0x10203040

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDITECT + NINDIRECT)

// 超级块描述了磁盘布局
struct superblock {
    uint magic;
    uint size;
    uint nblocks;
    uint ninodes;
    uint nlog;
    uint logstart;
    uint inodestart;
    uint bmapstart;
};

