#include <stdatomic.h>
#include <assert.h>
#include <errno.h>
#include "spin_lock.h"

/** 
 * atomic_compare_exchange_strong(ptr, oldval, newval)
 * atomically performs the equivalent of:
 * if (*ptr == *oldval)
 *     *ptr = newval;
 * It returns true if the test yielded true and *ptr was updated.
 */
static inline int atomic_cas(int *lock_addr, int expected, int desired) {
    return atomic_compare_exchange_strong(lock_addr, &expected, desired);
}

int thread_spin_init(mthread_spinlock_t *lock) {
    assert(lock);
    *lock = LOCK_NOT_ACQUIRED;
    return 0;
}

int thread_spin_lock(mthread_spinlock_t *lock) {
    assert(lock);
    while (!atomic_cas(lock, LOCK_NOT_ACQUIRED, LOCK_ACQUIRED));
    return 0;
}

int thread_spin_trylock(mthread_spinlock_t *lock) {
    assert(lock);
    return *lock ? EBUSY : 0;
}

int thread_spin_unlock(mthread_spinlock_t *lock) {
    assert(lock);
    atomic_cas(lock, LOCK_ACQUIRED, LOCK_NOT_ACQUIRED);
    return 0;
}