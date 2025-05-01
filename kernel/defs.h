#include "types.h"

struct spinlock;
struct cpu;
struct proc;

// console.c


// main.c
void            main(void);

// print.c
void            *memset(void *, int, uint);
void            printfinit(void);
int             printf(char *, ...);
void            panic(char *);

// proc.c
int             cpuid(void);
struct cpu      *mycpu(void);
struct proc     *myproc(void);
void            scheduler(void);
void            yield(void);
void            sleep(void *, struct spinlock *);
void            wakeup(void *);

// spinlock.c
void            initlock(struct spinlock *, char *);
void            push_off(void);
void            pop_off(void);
int             holding(struct spinlock *);
void            acquire(struct spinlock *);
void            release(struct spinlock *);

// uart.c
void            uartinit(void);