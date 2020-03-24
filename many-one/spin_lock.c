#include <stdatomic.h>
#include <assert.h>
#include <errno.h>
#include "spin_lock.h"

/** 
 * The atomic_compare_exchange_strong() macro stores a desired value into 
 * atomic variable object, only if the atomic variable is equal to its expected
 * value. Upon success, the macro returns true. Upon failure, the desired value 
 * is overwritten with the value of the atomic variable and false is returned.
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