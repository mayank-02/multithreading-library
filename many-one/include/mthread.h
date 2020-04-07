#ifndef _MTHREAD_H_
#define _MTHREAD_H_

#include <sys/types.h>
#include <stdint.h>
#include <setjmp.h>
#define MTHREAD_MAX_THREADS     128
#define MTHREAD_MIN_STACK       64 * 1024
#define MTHREAD_TCB_NAMELEN     128
#define MTHREAD_ATTR_DEFAULT    (NULL)
#define FALSE   (0)
#define TRUE    (!FALSE)
typedef enum thread_state {
    RUNNING = 0,
    READY,
    FINISHED,
    WAITING
} mthread_state_t;

typedef pid_t mthread_t;

typedef struct mthread {
    /* Thread ID */
    mthread_t tid;
    /* Thread State */
    mthread_state_t state;

    /* Start position of the code to be executed */
    void *(*start_routine) (void *);
    /* The argument passed to the function */
    void *arg;
    /* The result of the thread function */
    void *result;

    /* Stack handling*/
    void *stackaddr;
    size_t stacksize;

    /* Context of the thread */
    sigjmp_buf context;

    /* Name for debugging */
    char name[MTHREAD_TCB_NAMELEN];

    /* Detachment type */
    int joinable;
    /* TID to be joined to once finished */
    mthread_t joined_on;

    /* For signal handling */
    sigset_t sigpending;
} mthread;


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

typedef struct mthread_attr {
    /* Name for debugging */
    char a_name[MTHREAD_TCB_NAMELEN];

    /* Detachment type */
    int a_joinable;

    /* Stack handling */
    void *a_stackaddr;
    size_t a_stacksize;

} mthread_attr_t;

/* Thread attribute functions */
mthread_attr_t *mthread_attr_new(void);
int mthread_attr_init(mthread_attr_t *attr);
int mthread_attr_set(mthread_attr_t *attr, int field, ...);
int mthread_attr_get(mthread_attr_t *attr, int field, ...);
int mthread_attr_destroy(mthread_attr_t *attr);

/**
 * Perform any initialization needed. Should be called exactly
 * once, before any other mthread functions.
 */
int thread_init(void);

/**
 * Create a new thread starting at the routine given, which will
 * be passed arg. The new thread does not execute immediatly.
 */
int thread_create(mthread_t *thread, const mthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

/**
 * Wait until the specified thread has exited.
 * Returns the value returned by that thread's
 * start function.
 */
int thread_join(mthread_t thread, void **retval);

/**
 * Yield to scheduler
 */
void thread_yield(void);

/**
 * Exit the calling thread with return value retval.
 */
void thread_exit(void *retval);

/**
 * Send signal specified by sig to thread
 */
int thread_kill(mthread_t thread, int sig);

#endif