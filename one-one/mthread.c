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
#include "spin_lock.h"
#include "mthread.h"
#include "utils.h"

static size_t nproc;
static size_t stack_size;
static size_t page_size;
static queue * task_q;
static mthread_spinlock_t lock;

static inline int futex(int *uaddr, int futex_op, int val) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

static inline int arch_prctl(int code, unsigned long *addr) {
    return syscall(SYS_arch_prctl, code, addr);
}

static inline int tgkill(int tgid, int tid, int sig) {
    return syscall(SYS_tgkill, tgid, tid, sig);
}

static inline pid_t gettid(void) {
    return syscall(SYS_gettid);
}

/**
 * Obtain information about calling thread.
 */
static mthread *thread_self(void) {
    uint64_t ptr;

    int err = arch_prctl(ARCH_GET_FS, &ptr);
    if(err == -1)
        return NULL;

    return (mthread *)ptr;
}

static int thread_start(void *thread) {
    mthread *t = (mthread *)thread;

    if(sigsetjmp(t->context, 0) == 0)
        t->result = t->start_routine(t->arg);

    return 0;
}

int thread_init(void) {
    thread_spin_init(&lock);

    task_q = malloc(sizeof(queue));
    initialize(task_q);

    mthread *main = (mthread *) malloc(sizeof(mthread));
    main->start_routine = main->arg = main->result = NULL;
    main->base          = NULL;
    main->stack_size    = 0;
    main->tid           = gettid();
    enqueue(task_q, main);

    stack_size  = get_stack_size();
    nproc       = get_extant_process_limit();
    page_size   = get_page_size();

    return 0;
}

/**
 * Create a new thread starting at the routine given, which will
 * be passed arg.
 */
int thread_create(mthread_t *thread, mthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    thread_spin_lock(&lock);

    if(getcount(task_q) == nproc)
        return EAGAIN;

    if(start_routine == NULL)
        return EINVAL;

    mthread *t = (mthread *) malloc(sizeof(mthread));
    if(t == NULL)
        return EAGAIN;

    t->start_routine = start_routine;
    t->arg           = arg;
    t->detach_state  = (attr == NULL ? JOINABLE     : attr->a_detach_state);
    t->stack_size    = (attr == NULL ? stack_size   : attr->a_stack_size);
    t->base          = (attr == NULL ? NULL         : attr->a_base);
    if(t->base == NULL) {
        t->base = allocate_stack(t->stack_size);
        if(t->base == NULL) {
            free(t);
            return ENOMEM;
        }
    }

    if(attr == NULL)
        snprintf(t->name, MTHREAD_TCB_NAMELEN, "User%d", t->tid);
    else
        util_strncpy(t->name, attr->a_name, MTHREAD_TCB_NAMELEN);

    t->tid = clone(thread_start,
                   t->base + t->stack_size,
                   CLONE_VM | CLONE_FS | CLONE_FILES |
                   CLONE_SIGHAND |CLONE_THREAD | CLONE_SYSVSEM |
                   CLONE_SETTLS |CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID,
                   t,
                   &t->futex,
                   t,
                   &t->futex);
    if(t->tid == -1) {
        deallocate_stack(t->base, t->stack_size);
        free(t);
        return errno;
    }

    enqueue(task_q, t);

    *thread = t->tid;

    thread_spin_unlock(&lock);
    return 0;
}

/**
 * Wait until the specified thread has exited.
 * Returns the value returned by that thread's
 * start function.
 */
int thread_join(mthread_t thread, void **retval) {
    /* Search in queue for tcb corr to tid */
    thread_spin_lock(&lock);
    mthread *target = search_on_tid(task_q, thread);
    if(target == NULL)
        return ESRCH;

    if(target->detach_state == DETACHED || target->detach_state == JOINED)
        return EINVAL;


    target->detach_state = JOINED;
    thread_spin_unlock(&lock);

    int err = futex(&target->futex, FUTEX_WAIT, target->tid);
    if(err == -1 && errno != EAGAIN)
        return err;

    if(retval)
        *retval = target->result;

    return 0;
}

/**
 * Yield to scheduler
 */
void thread_yield(void) {
    sched_yield();
}

/**
 * Exit the calling thread with return value retval.
 */
void thread_exit(void *retval) {
    thread_spin_lock(&lock);
    mthread *self = thread_self();
    self->result = retval;
    thread_spin_unlock(&lock);
    siglongjmp(self->context, 1);
}

/**
 * Send signal specified by sig to thread
 */
int thread_kill(mthread_t thread, int sig) {
    if(sig == 0)
        return 0;

    pid_t tgid = getpid();
    int err = tgkill(tgid, thread, sig);
    if(err == -1)
        return errno;

    return 0;
}