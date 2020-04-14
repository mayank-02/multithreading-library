#ifndef _MTHREAD_H_
#define _MTHREAD_H_

#include "types.h"

enum {
    DETACHED,
    JOINABLE,
};

/// Operation to perform on thread attribute structure
enum {
    MTHREAD_ATTR_GET,
    MTHREAD_ATTR_SET
};

/// Attribute to read/write in thread attribute structure
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

/**
 * Perform any initialization needed. Should be called exactly
 * once, before any other mthread functions.
 */
int mthread_init(void);

/**
 * Create a new thread starting at the routine given, which will
 * be passed arg. The new thread does not execute immediatly.
 */
int mthread_create(mthread_t *thread, const mthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

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

/* Synchronisation Primitives */
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

#endif