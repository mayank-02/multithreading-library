#ifndef _MTHREAD_H_
#define _MTHREAD_H_

#include "types.h"

#define MTHREAD_ATTR_DEFAULT    NULL

enum {
    DETACHED,
    JOINABLE,
    JOINED
};

enum {
    MTHREAD_ATTR_GET,
    MTHREAD_ATTR_SET
};

enum {
    MTHREAD_ATTR_NAME,       /* RW [char *]    name of thread      */
    MTHREAD_ATTR_JOINABLE,   /* RW [int]       detachment type     */
    MTHREAD_ATTR_STACK_SIZE, /* RW [size_t]    stack               */
    MTHREAD_ATTR_STACK_ADDR  /* RW [void *]    stack lower         */
};

/* Thread attribute functions */
struct mthread_attr;
typedef struct mthread_attr mthread_attr_t;

mthread_attr_t *mthread_attr_new(void);

int mthread_attr_init(mthread_attr_t *attr);

int mthread_attr_set(mthread_attr_t *attr, int field, ...);

int mthread_attr_get(mthread_attr_t *attr, int field, ...);

int mthread_attr_destroy(mthread_attr_t *attr);

/* Thread control functions */

/**
 * Initialise the threading library
 */
int mthread_init(void);

/**
 * Create a new thread starting at the routine given, which will
 * be passed arg.
 */
int mthread_create(mthread_t *thread, mthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

/**
 * Wait until the specified thread has exited.
 * Returns the value returned by that thread's
 * start function.
 */
int mthread_join(mthread_t thread, void **retval);

/**
 * Yield to scheduler
 */
void mthread_yield(void);

/**
 * Exit the calling thread with return value retval.
 */
void mthread_exit(void *retval);

/**
 * Send signal specified by sig to thread
 */
int mthread_kill(mthread_t thread, int sig);

/**
 * Mark the thread as detached
 */
int mthread_detach(mthread_t thread);

/**
 * Compare Thread IDs
 */
int mthread_equal(mthread_t t1, mthread_t t2);


/* Synchronisation primitives */
struct mthread_spinlock;
typedef struct mthread_spinlock mthread_spinlock_t;

/*
 * Initialise the spinlock
 */
int mthread_spin_init(mthread_spinlock_t *lock);

/*
 * Lock the spinlock
 */
int mthread_spin_lock(mthread_spinlock_t *lock);

/*
 * Try locking the spinlock
 */
int mthread_spin_trylock(mthread_spinlock_t *lock);

/*
 * Unlock the spinlock
 */
int mthread_spin_unlock(mthread_spinlock_t *lock);

#define MTHREAD_MUTEX_INITIALIZER { 0 }
struct mthread_mutex;
typedef struct mthread_mutex mthread_mutex_t;

/*
 * Initialise the mutex
 */
int mthread_mutex_init(mthread_mutex_t *mutex);

/*
 * Try locking the mutex
 */
int mthread_mutex_trylock(mthread_mutex_t *mutex);

/*
 * Lock the mutex
 */
int mthread_mutex_lock(mthread_mutex_t *mutex);

/*
 * Unlock the mutex
 */
int mthread_mutex_unlock(mthread_mutex_t *mutex);

#define MTHREAD_COND_INITIALIZER { 0 , 0 }
struct mthread_cond;
typedef struct mthread_cond mthread_cond_t;

int mthread_cond_init(mthread_cond_t *cond);

int mthread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mutex);

int mthread_cond_signal(mthread_cond_t *cond);

#define MTHREAD_SEM_INITIALIZER { 0 }
struct mthread_sem;
typedef struct mthread_sem mthread_sem_t;

int mthread_sem_init(mthread_sem_t *sem, uint32_t initval);

int mthread_sem_wait(mthread_sem_t *sem);

int mthread_sem_post(mthread_sem_t *sem);

#endif