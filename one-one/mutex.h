#ifndef _MUTEX_H_
#define _MUTEX_H_

#define LOCKED_WAITING   (2u)
#define LOCKED           (1u)
#define UNLOCKED         (0u)

/* typedef struct mthread_mutex_t { 
    int            mx_state;
    mthread_t      mx_owner;
    unsigned long  mx_count;
} mthread_mutex_t; */

typedef int mthread_mutex_t;
int thread_mutex_init(mthread_mutex_t *mutex);
int thread_mutex_trylock(mthread_mutex_t *mutex);
int thread_mutex_lock(mthread_mutex_t *mutex);
int thread_mutex_unlock(mthread_mutex_t *mutex);

#endif