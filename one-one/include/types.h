#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <sys/types.h>
#include <setjmp.h>

/// Maximium length of name of thread
#define MTHREAD_TCB_NAMELEN     64

/// Thread Handle
typedef pid_t mthread_t;

/// Thread Control Block
typedef struct mthread {
    /// Thread ID
    mthread_t tid;

    /// Futex
    int32_t futex;

    /// Start position of the code to be executed
    void *(*start_routine) (void *);

    /// The argument passed to the function
    void *arg;

    /// The result of the thread function
    void *result;

    /// Padding for stack canary
    int64_t padding;

    /// Base pointer to stack
    void *stack_base;

    /// Size of stack
    size_t stack_size;

    /// Detachment type
    int detach_state;

    /// Name of process for debugging
    char name[MTHREAD_TCB_NAMELEN];

    /// For exiting safely
    sigjmp_buf context;
} mthread;

/// Thread Attribute Structure
struct mthread_attr {
    /// Name for debugging
    char a_name[MTHREAD_TCB_NAMELEN];

    /// Detachment type
    int a_detach_state;

    /// Base pointer to stack
    void *a_stack_base;

    /// Size of stack
    size_t a_stack_size;
};

/// States of a lock
#define CONTESTED   (2u)
#define LOCKED      (1u)
#define UNLOCKED    (0u)

/// Spinlock structure
struct mthread_spinlock {
    /// Value of spinlock
    int value;
};

/// Mutex structure
struct mthread_mutex {
    /// Value of mutex
    int value;
};

/// Condition Variable structure
struct mthread_cond {
    /// Current value of condition variable
    int value;

    /// Previous value of condition variable
    unsigned int previous;
};

/// Semaphore structure
struct mthread_sem {
    /// Value of semaphore
    int value;
};

#endif