#include "defs.h"
#include "sleeplock.h"

void initsleeplock(struct sleeplock *lk, char *name) {
    initlock(&(lk->lk), "sleep lock");
    lk->locked = 1;
    lk->name = name;
    lk->pid = 0;
}

