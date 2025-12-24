#ifndef LOCKS_H
#define LOCKS_H
#include <stdint.h>

typedef struct {
    volatile uint32_t lock;
    uintptr_t flags;
} SpinLock_t;

#define SPINLOCK_INIT { .lock = 0, .flags = 0 }
void SpinLock(SpinLock_t* l);
void SpinUnlock(SpinLock_t* l);
void SpinLockIrq(SpinLock_t* l);
void SpinUnlockIrq(SpinLock_t* l);
#endif