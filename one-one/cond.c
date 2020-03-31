#define _GNU_SOURCE
#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include "cond.h"

static inline int futex(int *uaddr, int futex_op, int val) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

int thread_cond_init(mthread_cond_t *cond) {
    atomic_init(&cond->value, 0);
    atomic_init(&cond->previous, 0);
    return 0;
}

int thread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mutex) {
    int value = atomic_load(&cond->value);

    atomic_store(&cond->previous, value);

    thread_mutex_unlock(mutex);
    futex(&cond->value, FUTEX_WAIT_PRIVATE, value);
    thread_mutex_lock(mutex);

    return 0;
}

int thread_cond_signal(mthread_cond_t *cond) {
    unsigned value = 1u + atomic_load(&cond->previous);

    atomic_store(&cond->value, value);

    futex(&cond->value, FUTEX_WAKE_PRIVATE, 1);
    return 0;
}