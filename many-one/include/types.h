#ifndef _TYPES_H_
#define _TYPES_H_

#include <sys/types.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
/// Maximum threads that can be created
#define MTHREAD_MAX_THREADS     128

/// Minimum stack for each thread
#define MTHREAD_MIN_STACK       64 * 1024

/// Maximium length of name of thread
#define MTHREAD_TCB_NAMELEN     128

/// Default attribute of thread
#define MTHREAD_ATTR_DEFAULT    (NULL)

#define FALSE   (0)
#define TRUE    (!FALSE)

/// States of a thread
typedef enum mthread_state {
    RUNNING = 0,
    READY,
    FINISHED,
    WAITING
} mthread_state_t;

/// Thread Handle
typedef pid_t mthread_t;

/// Thread Control Block
typedef struct mthread {
    /// Thread ID
    mthread_t tid;

    /// Thread State
    mthread_state_t state;

    /// Start position of the code to be executed
    void *(*start_routine) (void *);

    /// Argument passed to the function
    void *arg;

    /// Result of the thread function
    void *result;

    /// Stack Base
    void *stackaddr;

    /// Stack Size
    size_t stacksize;

    /// Context of the thread
    sigjmp_buf context;

    /// Name for debugging
    char name[MTHREAD_TCB_NAMELEN];

    /// Detachment type
    int joinable;

    /// TID to be joined to once finished
    mthread_t joined_on;

    /// For signal handling
    sigset_t sigpending;
} mthread;

/// Thread Attribute Structure
struct mthread_attr {
    /// Name for debugging
    char a_name[MTHREAD_TCB_NAMELEN];

    /// Detachment type
    int a_joinable;

    /// Stack Base
    void *a_stackaddr;

    /// Stack Size
    size_t a_stacksize;
};

/// States of a spinlock
#define LOCKED     (1u)
#define UNLOCKED   (0u)

/// Spinlock structure
struct mthread_spinlock {
    /// Value of spinlock
    int value;
};

#endif