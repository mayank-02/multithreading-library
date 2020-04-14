/**
 * @file sem.c
 * @brief Semaphore Synchronisation Primitive
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
 * @brief Initialise the semaphore
 * @param[in,out] sem Pointer to semaphore
 * @param[in,out] initval Value to be initialised to
 * @return On success, returns 0
 */
int mthread_sem_init(mthread_sem_t *sem, uint32_t initval) {
    assert(sem);
    atomic_init(&sem->value, initval);
    return 0;
}

/**
 * @brief Decrements (locks) the semaphore
 * @param[in,out] sem Pointer to semaphore
 * @note If the semaphore currently has the value zero, then the
 * call blocks  until it becomes possible to perform the decrement
 * @return On success, returns 0
 */
int mthread_sem_wait(mthread_sem_t *sem) {
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

/**
 * @brief Increments (unlocks) the semaphore
 * @param[in,out] sem Pointer to semaphore
 * @return On success, returns 0
 */
int mthread_sem_post(mthread_sem_t *sem) {
    assert(sem);
    atomic_fetch_add_explicit(&sem->value, 1, memory_order_release);
    futex(&sem->value, FUTEX_WAKE_PRIVATE, 1);
    return 0;
}