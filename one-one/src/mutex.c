/**
 * @file mutex.c
 * @brief Mutex Synchronisation Primitive
 * @author Mayank Jain
 * @bug No known bugs
 */

#define _GNU_SOURCE
#include <stdatomic.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>
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
 * @brief Atomic Compare and Exchange
 * Atomically performs the equivalent of:
 * if (*ptr == *oldval)
 *     *ptr = newval;
 * @param[in,out] lock_addr Pointer to lock
 * @param[in] expected Expected value of lock
 * @param[in] desirec Desired value of lock
 * @return Returns the expected value
 */
static inline int cmpxchg(int *lock_addr, int expected, int desired) {
  int *ep = &expected;
  atomic_compare_exchange_strong(lock_addr, ep, desired);
  return *ep;
}

/**
 * @brief Initialise the mutex
 * @param[in,out] mutex Pointer to mutex
 * @return Always returns 0
 */
int mthread_mutex_init(mthread_mutex_t *mutex) {
    assert(mutex);
    mutex->value = UNLOCKED;
    return 0;
}

/**
 * @brief Lock the mutex
 * @param[in,out] mutex Pointer to mutex
 * @note If the mutex is already locked by another thread, the
 * calling thread is suspended until the mutex is unlocked.
 * @return On success, returns 0
 */
int mthread_mutex_lock(mthread_mutex_t *mutex) {
    assert(mutex);
    
    int c = cmpxchg(&mutex->value, UNLOCKED, LOCKED);
    
    /*
     * If the lock was previously unlocked, there's nothing else for us to do.
     * Otherwise, we'll probably have to wait.
     */
    if(c != 0) {
        do {
            
            /* 
             * If the mutex is locked, we signal that we're waiting by setting 
             * the mutex->value to 2. A shortcut checks is it's 2 already and 
             * avoids the atomic operation in this case.
             */
            if(c == 2 || cmpxchg(&mutex->value, LOCKED, CONTESTED) != 0) {
                
                /*
                 * Here we have to actually sleep, because the mutex is actually
                 * locked. Note that it's not necessary to loop around this 
                 * syscall;
                 * A spurious wakeup will do no harm since we only exit the 
                 * do...while loop when mutex->value is indeed 0. 
                 */
                futex(&mutex->value, FUTEX_WAIT, CONTESTED);
            }
            
            /*
             * We're here when either:
             * (a) the mutex was in fact unlocked (by an intervening thread).
             * (b) we slept waiting for the atom and were awoken.
             * 
             * So we try to lock the atom again. We set the state to 2 because 
             * we can't be certain there's no other thread at this exact point. 
             * So we prefer to err on the safe side.
             */ 
        } while((c = cmpxchg(&mutex->value, UNLOCKED, CONTESTED)) != 0);
    }
    return 0;
}

/**
 * @brief Try locking the mutex
 * @param[in,out] mutex Pointer to mutex
 * @note does not block the calling  thread  if  the  mutex  is
 * already locked  by  another  thread
 * @return On locking returns 0, else EBUSY
 */
int mthread_mutex_trylock(mthread_mutex_t *mutex) {
    assert(mutex);
    return atomic_cas(&mutex->value, UNLOCKED, UNLOCKED) ? 0 : EBUSY;
}

/**
 * @brief Unlock the mutex
 * @param[in,out] mutex Pointer to mutex
 * @return On success, returns 0
 */
int mthread_mutex_unlock(mthread_mutex_t *mutex) {
    assert(mutex);
    if(atomic_fetch_sub(&mutex->value, 1) != 1) {
        atomic_store(&mutex->value, UNLOCKED);
        futex(&mutex->value, FUTEX_WAKE, 1);
    }
    return 0;
}