#ifndef _MTHREAD_H_
#define _MTHREAD_H_

#include <sys/types.h>
#include <setjmp.h>
#define MAX_THREADS     128
#define MIN_STACK       64 * 1024

typedef enum thread_state {
    RUNNING = 0, READY, SUSPENDED, FINISHED, WAITING
} thread_state;

typedef pid_t mthread_t;

typedef struct mthread {
    /* Thread ID */
    mthread_t tid;
    /* Thread State */
    thread_state state;

    /* Start position of the code to be executed */ 
    void *(*start_routine) (void *);
    /* The argument passed to the function */
    void *arg;
    /* The result of the thread function */
    void *result;
    
    /* Context of the thread */
    sigjmp_buf context;
    char stack[MIN_STACK];

    /* TID to be joined to once finished */
    mthread_t joined_on;
    /* TID for whom we are waiting for */
    mthread_t wait_for;
} mthread;

/** 
 * Perform any initialization needed. Should be called exactly
 * once, before any other mthread functions.
 */
int thread_init(void);

/** 
 * Create a new thread starting at the routine given, which will
 * be passed arg. The new thread does not execute immediatly.
 */
int thread_create(mthread_t *thread, void *(*start_routine)(void *), void *arg);

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