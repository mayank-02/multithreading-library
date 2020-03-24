#ifndef _MTHREAD_H_
#define _MTHREAD_H_

#include <sys/types.h>
#include <ucontext.h>
#include <setjmp.h>
#define MAX_THREADS     128
#define MIN_STACK       64 * 1024

typedef enum thread_state {
    RUNNING = 0, READY, SUSPENDED, FINISHED, SLEEPING, BLOCKED_JOIN, DEAD
} thread_state;

typedef unsigned long int mthread_t;

typedef struct mthread {
    /* Thread ID */
    unsigned long int tid;
    /* Thread State */
    thread_state state;

    /* Start position of the code to be executed */ 
    void *(*start_routine) (void *);
    /* The argument passed to the function */
    void *arg;
    /* The result of the thread function */
    void *result;
    
    /* Context of the thread */
    jmp_buf context;
    char stack[MIN_STACK];

    /* The TID of the thread to be joined to once finished */
    long int joined_on;
    /* The TID of the thread for whom we are waiting */
    long int wait_for;
} mthread;

/* Perform any initialization needed. Should be called exactly
 * once, before any other mthread functions.
 */
int thread_init(void);

/* Create a new thread starting at the routine given, which will
 * be passed arg. The new thread does not necessarily execute immediatly
 * (as in, thread_create shouldn't force a switch to the new thread).
 * If the thread will be joined, the joinable flag should be set. 
 * Otherwise, it should be 0.
 */
int thread_create(mthread_t *thread, void *(*start_routine)(void *), void *arg);

/* Wait until the specified thread has exited.
 * Returns the value returned by that thread's
 * start function.  Results are undefined if
 * if the thread was not created with the joinable
 * flag set or if it has already been joined.
 */
int thread_join(mthread_t thread, void **retval);

/* Exit the calling thread with return value ret. */
void thread_exit(void *retval);

int thread_kill(mthread_t thread, int sig);

#endif