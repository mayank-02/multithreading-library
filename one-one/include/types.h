#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <sys/types.h>
#include <setjmp.h>

#define MTHREAD_TCB_NAMELEN     64

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

    /* Padding for stack canary */
    int64_t padding;

    /* Base pointer to stack */
    void *stack_base;

    /* Size of stack */
    size_t stack_size;

    /* Detachment type */
    int detach_state;

    /* Name of process for debugging */
    char name[MTHREAD_TCB_NAMELEN];

    /* For exiting safely */
    sigjmp_buf context;
} mthread;


struct mthread_attr {
    /* Name for debugging */
    char a_name[MTHREAD_TCB_NAMELEN];

    /* Detachment type */
    int a_detach_state;

    /* Stack handling */
    void *a_stack_base;
    size_t a_stack_size;

};

#define CONTESTED   (2u)
#define LOCKED      (1u)
#define UNLOCKED    (0u)

struct mthread_spinlock {
    int value;
};

struct mthread_mutex {
    int value;
};

struct mthread_cond {
    int value;
    unsigned int previous;
};

struct mthread_sem {
    int value;
};

#endif