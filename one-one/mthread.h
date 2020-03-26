#ifndef _MTHREAD_H_
#define _MTHREAD_H_

#include <stdint.h>
#include <sys/types.h>
typedef pid_t mthread_t;

typedef struct mthread {
    /* Thread ID */
    mthread_t tid;

    /* Start position of the code to be executed */ 
    void *(*start_routine) (void *);
    /* The argument passed to the function */
    void *arg;
    /* The result of the thread function */
    void *result;
    
    /* Base pointer to stack */
    void *base;
    /* Size of stack */
    uint64_t stack_size;

    /* Has someone joined on it? */
    int joined;
    /* Futex */
    uint32_t condition;
} mthread;

/** 
 * Create a new thread starting at the routine given, which will
 * be passed arg.
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