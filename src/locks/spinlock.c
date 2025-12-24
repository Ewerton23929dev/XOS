#include <locks/spinlock.h>

void SpinLockIrq(SpinLock_t* l)
{
    while (__atomic_test_and_set(&l->lock,__ATOMIC_ACQUIRE)) {
        asm volatile("pause");
    }
}
void SpinUnlockIrq(SpinLock_t* l)
{
    __atomic_clear(&l->lock,__ATOMIC_RELEASE);
}
void SpinLock(SpinLock_t* l)
{
    asm volatile("pushf; pop %0; cli" : "=r"(l->flags) :: "memory");
    while (__atomic_test_and_set(&l->lock,__ATOMIC_ACQUIRE)) {
        asm volatile("pause");
    }
}
void SpinUnlock(SpinLock_t* l)
{
    __atomic_clear(&l->lock,__ATOMIC_RELEASE);
    asm volatile("push %0; popf" :: "r"(l->flags) : "memory");
}