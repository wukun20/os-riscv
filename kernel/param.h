#define MAXVA   (1L << (39- 1)) // 最大虚拟地址
#define PGSIZE  4*1024          // 页大小（B）
#define PGSHIFT 12              // 页内偏移量（b）

#define MEMSIZE 128*1024*1024   // 内存大小（B）
#define NCPU    1               // 核心数量
#define NPROC   64              // 进程数量
#define NDEV    10              // 主设备数量
