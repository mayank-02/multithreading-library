/**
 * @file cond.c
 * @brief Condition Variable Synchronisation Primitive
 * @author Mayank Jain
 * @bug No known bugs
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <assert.h>
#include <stdatomic.h>
#include "mthread.h"

/**
 * @brief Fast user-space locking
 * @param[in] uaddr Pointer to futex word
 * @param[in] futex_op Operation to be performed
 * @param[in] val Expected value of the futex word
 * @return 0 on success; -1 on error
 */
static inline int futex(int *uaddr, int futex_op, int val) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

/**
 * @brief Initialise the condition variable
 * @param[in,out] cond Pointer to condition variable
 * @return Always returns 0
 */
int mthread_cond_init(mthread_cond_t *cond) {
    assert(cond);

    atomic_init(&cond->value, 0);
    atomic_init(&cond->previous, 0);

    return 0;
}

/**
 * @brief Atomically unlocks the mutex and waits for CV to be signaled
 * @param[in,out] cond Pointer to condition variable
 * @param[in,out] mutex Pointer to associated mutex
 * @note The thread execution is suspended and does not consume any
 * CPU time until the condition variable is signaled.
 * @return On success, returns 0
 */
int mthread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mutex) {
    assert(cond && mutex);

    int value = atomic_load(&cond->value);
    atomic_store(&cond->previous, value);

    mthread_mutex_unlock(mutex);
    futex(&cond->value, FUTEX_WAIT_PRIVATE, value);
    mthread_mutex_lock(mutex);

    return 0;
}

/**
 * @brief Restarts one of the threads that are waiting
 * on the condition variable
 * @param[in,out] cond Pointer to condition variable
 * @return On success, returns 0
 */
int mthread_cond_signal(mthread_cond_t *cond) {
    assert(cond);

    unsigned value = 1u + atomic_load(&cond->previous);
    atomic_store(&cond->value, value);

    futex(&cond->value, FUTEX_WAKE_PRIVATE, 1);

    return 0;
}