#ifndef _MTHREAD_H_
#define _MTHREAD_H_

#include <stdint.h>
#include <sys/types.h>
#include <setjmp.h>

#define MTHREAD_TCB_NAMELEN     128
#define MTHREAD_ATTR_DEFAULT    NULL
typedef pid_t mthread_t;

typedef struct mthread {
    /* Thread ID */
    mthread_t tid;

    /* Futex */
    int32_t futex;
    
    /* Start position of the code to be executed */ 
    void *(*start_routine) (void *);
    
    /* The argument passed to the function */
    void *arg;
    
    /* The result of the thread function */
    void *result;
    
    /* Base pointer to stack */
    void *base;
    
    /* Size of stack */
    size_t stack_size;

    /* Has someone joined on it? */
    int detach_state;
    
    /* Name of process for debugging */
    char name[MTHREAD_TCB_NAMELEN];

    /* For exiting safely */
    sigjmp_buf context;
} mthread;

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

typedef struct mthread_attr {
    /* Name for debugging */
    char a_name[MTHREAD_TCB_NAMELEN];

    /* Detachment type */
    int a_detach_state;

    /* Stack handling */
    void *a_base;
    size_t a_stack_size;

} mthread_attr_t;

/* Thread attribute functions */
mthread_attr_t *mthread_attr_new(void);
int mthread_attr_init(mthread_attr_t *attr);
int mthread_attr_set(mthread_attr_t *attr, int field, ...);
int mthread_attr_get(mthread_attr_t *attr, int field, ...);
int mthread_attr_destroy(mthread_attr_t *attr);

int thread_init(void);
/** 
 * Create a new thread starting at the routine given, which will
 * be passed arg.
 */
int thread_create(mthread_t *thread, mthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

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