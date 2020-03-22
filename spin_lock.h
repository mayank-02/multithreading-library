#ifndef _SPIN_LOCK_H_
#define _SPIN_LOCK_H_

#define LOCK_ACQUIRED     (1u)
#define LOCK_NOT_ACQUIRED (0u)

typedef int mthread_spinlock_t;

int thread_spin_init(mthread_spinlock_t *lock);

int thread_spin_lock(mthread_spinlock_t *lock);

int thread_spin_trylock(mthread_spinlock_t *lock);

int thread_spin_unlock(mthread_spinlock_t *lock);

#endif