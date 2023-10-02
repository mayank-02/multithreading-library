/**
 * @file mthread.c
 * @brief Thread control functions
 * @author Mayank Jain
 * @bug No known bugs
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
#include "queue.h"
#include "stack.h"
#include "mthread.h"
#include "utils.h"

static size_t   nproc;          ///< Number of extant processes allowed
static size_t   stack_size;     ///< Stack size
static size_t   page_size;      ///< Page size
static queue *  task_q;         ///< Queue containing tasks for all threads
static mthread_spinlock_t lock; ///< Lock for task queue

/**
 * @brief Fast user-space locking
 * @param[in] uaddr Pointer to futex word
 * @param[in] futex_op Operation to be performed
 * @param[in] val Expected value of the futex word
 * @return 0 on success; -1 on error
 */
static inline int futex(int *uaddr, int futex_op, int val) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

/**
 * @brief Get/Set architecture-specific thread state
 * @param[in] code Subfunction
 * @param[in/out] Address of value to "set" or "get"
 * @return On success, returns 0; on error, -1 is returned, and errno is set to indicate the error.
 */
static inline int arch_prctl(int code, unsigned long *addr) {
    return syscall(SYS_arch_prctl, code, addr);
}

/**
 * @brief Sends a signal to a thread
 * @param[in] tgid Thread Group ID
 * @param[in] tid Thread ID
 * @param[in] sig Signal number
 * @return 0 on success; -1 on error
 */
static inline int tgkill(int tgid, int tid, int sig) {
    return syscall(SYS_tgkill, tgid, tid, sig);
}

/**
 * @brief Cleans up all malloc(3)ed and mmap(3)ed regions
 */
static void cleanup_handler(void) {
    mthread *t;
    int n = getcount(task_q);
    while(n--) {
        t = dequeue(task_q);
        if(t->detach_state == JOINED) {
            deallocate_stack(t->stack_base, t->stack_size);
            free(t);
        }
    }
    free(task_q);
}

/**
 * @brief Obtain information about calling thread
 * @return Pointer to thread handle on success, and NULL on failure
 */
static mthread *mthread_self(void) {
    uint64_t ptr;
    pid_t tid = gettid();
    if(tid == getpid()){
        return NULL;
    }

    int err = arch_prctl(ARCH_GET_FS, &ptr);
    if(err == -1)
        return NULL;

    return (mthread *)ptr;
}

/**
 * @brief Wrapper around user start function
 * @return On success, returns 0
 */
static int mthread_start(void *thread) {
    mthread *t = (mthread *)thread;

    if(sigsetjmp(t->context, 0) == 0)
        t->result = t->start_routine(t->arg);

    return 0;
}

/**
 * @brief Initialise the mthread library
 * @note It has to be the first mthread API function call in an application,
 * and is mandatory.
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_init(void) {
    mthread_spin_init(&lock);

    task_q = calloc(1, sizeof(queue));
    initialize(task_q);

    atexit(cleanup_handler);

    mthread *main_thread = (mthread *) calloc(1, sizeof(mthread));
    main_thread->start_routine = main_thread->arg = main_thread->result = NULL;
    main_thread->detach_state  = JOINABLE;
    main_thread->stack_base    = NULL;
    main_thread->stack_size    = 0;
    main_thread->tid           = gettid();
    enqueue(task_q, main_thread);

    stack_size  = get_stack_size();
    nproc       = get_extant_process_limit();
    page_size   = get_page_size();

    return 0;
}

/**
 * @brief Create a new thread
 * @param[in] thread Pointer to thread handle
 * @param[in] attr Pointer to attribute object
 * @param[in] start_routine Start function of the thread
 * @param[in] arg Arguments to be passed to the start function
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_create(mthread_t *thread, mthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    mthread_spin_lock(&lock);

    if(getcount(task_q) == nproc) {
        mthread_spin_unlock(&lock);
        return EAGAIN;
    }

    if(start_routine == NULL) {
        mthread_spin_unlock(&lock);
        return EINVAL;
    }

    mthread *t = (mthread *) calloc(1, sizeof(mthread));
    if(t == NULL) {
        mthread_spin_unlock(&lock);
        return EAGAIN;
    }

    t->start_routine = start_routine;
    t->arg           = arg;
    t->detach_state  = (attr == NULL ? JOINABLE     : attr->a_detach_state);
    t->stack_size    = (attr == NULL ? stack_size   : attr->a_stack_size);
    t->stack_base    = (attr == NULL ? NULL         : attr->a_stack_base);
    if(t->stack_base == NULL) {
        t->stack_base = allocate_stack(t->stack_size);
        if(t->stack_base == NULL) {
            free(t);
            mthread_spin_unlock(&lock);
            return ENOMEM;
        }
    }

    if(attr == NULL)
        snprintf(t->name, MTHREAD_TCB_NAMELEN, "User%d", t->tid);
    else
        util_strncpy(t->name, attr->a_name, MTHREAD_TCB_NAMELEN);

    t->tid = clone(mthread_start,
                   t->stack_base + t->stack_size,
                   CLONE_VM | CLONE_FS | CLONE_FILES |
                   CLONE_SIGHAND |CLONE_THREAD | CLONE_SYSVSEM |
                   CLONE_SETTLS |CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID,
                   t,
                   &t->futex,
                   t,
                   &t->futex);
    if(t->tid == -1) {
        deallocate_stack(t->stack_base, t->stack_size);
        free(t);
        mthread_spin_unlock(&lock);
        return errno;
    }

    enqueue(task_q, t);

    *thread = t->tid;

    mthread_spin_unlock(&lock);
    return 0;
}

/**
 * @brief Join with a terminated thread
 * @param[in] thread Handle of thread to wait for
 * @param[in] retval To save the exit status of the target thread
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_join(mthread_t thread, void **retval) {
    mthread_spin_lock(&lock);

    mthread *target = search_on_tid(task_q, thread);
    if(target == NULL) {
        mthread_spin_unlock(&lock);
        return ESRCH;
    }

    if(target->detach_state == DETACHED || target->detach_state == JOINED) {
        mthread_spin_unlock(&lock);
        return EINVAL;
    }

    target->detach_state = JOINED;
    mthread_spin_unlock(&lock);

    int err = futex(&target->futex, FUTEX_WAIT, target->tid);
    if(err == -1 && errno != EAGAIN)
        return err;

    if(retval)
        *retval = target->result;

    return 0;
}

/**
 * @brief Yield the processor
 */
void mthread_yield(void) {
    sched_yield();
}

/**
 * @brief Terminate calling thread
 * @param[in] retval Return value of the thread
 * @return Does not return to the caller
 * @note Performing a return from the start funciton of any thread results in
 * an implicit call to mthread_exit()
 */
void mthread_exit(void *retval) {
    mthread_spin_lock(&lock);
    mthread *self = mthread_self();
    if(self == NULL) {
        mthread_spin_unlock(&lock);
        return;
    }

    self->result = retval;
    mthread_spin_unlock(&lock);
    siglongjmp(self->context, 1);
}

/**
 * @brief Send a signal to a thread
 * @param[in] thread Thread handle of thread to which signal needs to be sent
 * @param[in] sig    Signal number corresponding to signal
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_kill(mthread_t thread, int sig) {
    if(sig == 0)
        return 0;

    pid_t tgid = getpid();
    int err = tgkill(tgid, thread, sig);
    if(err == -1)
        return errno;

    return 0;
}

/**
 * @brief Detach a thread
 * @param[in] thread Thread handle of thread to be detached
 * @return On success, returns 0; on error, it returns an error number
 * @note Once a thread has been detached, it can't be joined with mthread_join * or be made joinable again.
 */
int mthread_detach(mthread_t thread) {
    mthread_spin_lock(&lock);

    mthread *target = search_on_tid(task_q, thread);

    if(target == NULL) {
        mthread_spin_unlock(&lock);
        return ESRCH;
    }

    if(target->detach_state == JOINED) {
        mthread_spin_unlock(&lock);
        return EINVAL;
    }

    target->detach_state = DETACHED;
    mthread_spin_unlock(&lock);

    return 0;
}

/**
 * @brief Compare Thread IDs
 * @param[in] t1 Thread handle of thread 1
 * @param[in] t2 Thread handle of thread 2
 * @return 0 on success, and non-zero on failure
 */
int mthread_equal(mthread_t t1, mthread_t t2) {
    return t1 - t2;
}
