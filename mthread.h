#ifndef _MTHREADS_H_
#define _MTHREADS_H_

#include <sys/types.h>
#include <ucontext.h>
#include <setjmp.h>
#define MAX_THREADS     128
#define MIN_STACK       64 * 1024
#define TIMER           (useconds_t) 10000
typedef enum thread_state {
    RUNNING = 0, READY, SUSPENDED, FINISHED, SLEEPING, BLOCKED_JOIN, DEAD
} thread_state;

typedef struct timeval pth_time_t;
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
    // ucontext_t *context;

    /* Whether thread is joinable or detached */
    int joinable;
    /* The TID of the thread to be joined to once finished */
    long int joined_on;
    /* The TID of the thread for whom we are waiting */
    long int wait_for;
    /* Joining argument */
    void *join_arg;

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

// int thread_lock(mthread_lock_t *lock); // a spinlock

// int thread_unlock(mthread_lock_t *lock);  // spin-unlock

int thread_kill(mthread_t thread, int sig);

/* Signal handler for SIGALRM signal
 * Schedules next READY thread by swapping context at user level
 */
void scheduler(int signum);

void wrapper(void);
#endif