#define _GNU_SOURCE
#include <stdatomic.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "mutex.h"

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

static inline int futex(int *uaddr, int futex_op, int val) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

static inline int cmpxchg(int *lock_addr, int expected, int desired) {
  int *ep = &expected;
  atomic_compare_exchange_strong(lock_addr, ep, desired);
  return *ep;
}

int thread_mutex_init(mthread_mutex_t *mutex) {
    assert(mutex);
    *mutex = UNLOCKED;
    return 0;
}

int thread_mutex_lock(mthread_mutex_t *mutex) {
    assert(mutex);
    int c = cmpxchg(mutex, UNLOCKED, LOCKED);
    if(c != 0) {
        do {
            if(c == 2 || cmpxchg(mutex, LOCKED, LOCKED_WAITING) != 0) {
                futex(mutex, FUTEX_WAIT, LOCKED_WAITING);
            }
        } while((c = cmpxchg(mutex, UNLOCKED, LOCKED_WAITING)) != 0);
    }
    return 0;
}

int thread_mutex_trylock(mthread_mutex_t *mutex) {
    assert(mutex);
    return atomic_compare_exchange_strong(mutex, UNLOCKED, UNLOCKED) ? 0 : EBUSY;
}

int thread_mutex_unlock(mthread_mutex_t *mutex) {
    assert(mutex);
    if(atomic_fetch_sub(mutex, 1) != 1) {
        atomic_store(mutex, UNLOCKED);
        futex(mutex, FUTEX_WAKE, 1);
    }
    return 0;
}