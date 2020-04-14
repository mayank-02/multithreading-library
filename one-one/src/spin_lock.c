/**
 * @file spin_lock.c
 * @brief Spinlock Synchronisation Primitive
 * @author Mayank Jain
 * @bug No known bugs
 */

#include <stdatomic.h>
#include <assert.h>
#include <errno.h>
#include "mthread.h"

/**
 * @brief Atomic Compare and Swap
 * @param[in,out] lock_addr Pointer to lock
 * @param[in] expected Expected value of lock
 * @param[in] desirec Desired value of lock
 * @return Upon success, the macro returns true. Upon failure, the desired
 * value is overwritten with the value of the atomic variable and false is
 * returned.
 */
static inline int atomic_cas(int *lock_addr, int expected, int desired) {
    return atomic_compare_exchange_strong(lock_addr, &expected, desired);
}

/**
 * @brief Initialise the spinlock
 * @param[out] lock Pointer to the spinlock
 * @note Reinitialization of a lock held by other threads may lead to
 * unexpected results, hence this function must be called only once
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_spin_init(mthread_spinlock_t *lock) {
    assert(lock);
    lock->value = UNLOCKED;
    return 0;
}

/**
 * @brief Lock a spinlock
 * @param[in,out] lock Pointer to the spinlock
 * @note The call is blocking and will return only if the lock is acquired
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_spin_lock(mthread_spinlock_t *lock) {
    assert(lock);
    while (!atomic_cas(&lock->value, UNLOCKED, LOCKED));
    return 0;
}

/**
 * @brief Try locking a spinlock
 * @param[in,out] lock Pointer to the spinlock
 * @note The call returns immediately if acquiring lock fails
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_spin_trylock(mthread_spinlock_t *lock) {
    assert(lock);
    if(atomic_cas(&lock->value, UNLOCKED, LOCKED))
        return 0;

    return EBUSY;
}

/**
 * @brief Unlock a spinlock
 * @param[in,out] lock Pointer to the spinlock
 * @note Calling mthread_spin_unlock() on a lock that is not held by the
 * caller results in undefined behavior.
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_spin_unlock(mthread_spinlock_t *lock) {
    assert(lock);
    atomic_cas(&lock->value, LOCKED, UNLOCKED);
    return 0;
}