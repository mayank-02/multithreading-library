#define _GNU_SOURCE
#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <assert.h>
#include "sem.h"

static inline int futex(int *uaddr, int futex_op, int val) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

int thread_sem_init(mthread_sem_t *sem, uint32_t initval) {
    assert(sem);
    atomic_init(&sem->value, initval);
    return 0;
}

int thread_sem_wait(mthread_sem_t *sem) {
    assert(sem);
    uint32_t value = 1;

    while(!atomic_compare_exchange_weak_explicit(&sem->value,
                                                    &value, value - 1,
                                                    memory_order_acquire,
                                                    memory_order_relaxed)) {
        if(value == 0) {
            futex(&sem->value, FUTEX_WAIT_PRIVATE, 0);
            value = 1;
        }
    }

    return 0;
}

int thread_sem_post(mthread_sem_t *sem) {
    assert(sem);
    atomic_fetch_add_explicit(&sem->value, 1, memory_order_release);
    futex(&sem->value, FUTEX_WAKE_PRIVATE, 1);
    return 0;
}