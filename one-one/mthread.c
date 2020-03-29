#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <linux/futex.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <string.h>
#include "queue.h"
#include "spin_lock.h"
#include "mthread.h"

#define print(str) write(1, str, strlen(str))

static int initialized = 0;
static uint64_t nproc;
static uint64_t stack_size;
static uint64_t page_size;
static queue * task_q;
static mthread_spinlock_t lock;

static uint64_t get_extant_process_limit(void) {
    struct rlimit limit;
    getrlimit(RLIMIT_NPROC, &limit);
    // printf ("The current maximum number of processes is %d.\n", (int) curr_limits.rlim_cur);
    // printf ("The hard limit on the number of processes is %d.\n", (int) curr_limits.rlim_max);
    return limit.rlim_cur;
}

static uint64_t get_stack_size(void) {
    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    // printf("Soft limit: %ju bytes\n", (uintmax_t)limit.rlim_cur);
    // printf("Hard limit: %ju bytes\n", (uintmax_t)limit.rlim_max);
    return limit.rlim_cur;
}

static uint64_t get_page_size() {
    return sysconf(_SC_PAGESIZE);
}

static void * allocate_stack(uint64_t stack_size) {
    void *base = mmap(NULL,
                      stack_size + page_size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
                      -1,
                      0);
    if(base == MAP_FAILED)
        return NULL;

    if(mprotect(base, page_size, PROT_NONE) == -1) {
        munmap(base, stack_size + page_size);
        return NULL;
    }

    /* Assume stack grows downward */
    return base + page_size;
}

static void deallocate_stack(void *base, size_t stack_size) {
    munmap(base - page_size, stack_size + page_size);
}

static inline int futex(int *uaddr, int futex_op, int val) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

static int arch_prctl(int code, unsigned long *addr) {
    return syscall(SYS_arch_prctl, code, addr);
}

static int tgkill(int tgid, int tid, int sig) {
    return syscall(SYS_tgkill, tgid, tid, sig);
}

static pid_t gettid(void) {
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

/**
 * Create a new thread starting at the routine given, which will
 * be passed arg.
 */
int thread_create(mthread_t *thread, void *(*start_routine)(void *), void *arg) {
    if(!initialized) {
        thread_spin_init(&lock);
        task_q = malloc(sizeof(queue));
        initialize(task_q);

        mthread *main = (mthread *) malloc(sizeof(mthread));
        main->start_routine = main->arg = main->result = NULL;
        main->tid = gettid();
        enqueue(task_q, main);

        stack_size = get_stack_size();
        nproc = get_extant_process_limit();
        page_size = get_page_size();

        initialized = 1;
    }
    thread_spin_lock(&lock);
    if(getcount(task_q) == nproc) {
        return EAGAIN;
    }

    mthread *t = (mthread *) malloc(sizeof(mthread));
    if(t == NULL) {
        /* printf("Insufficient resources to create another thread.\n"); */
        return EAGAIN;
    }
    if(start_routine == NULL) {
        return -1;
    }
    t->start_routine = start_routine;
    t->arg = arg;
    t->joined = 0;
    t->stack_size = stack_size;
    t->base = allocate_stack(t->stack_size);
    if(t->base == NULL) {
        free(t);
        return -1;
    }

    t->tid = clone(thread_start,
                   t->base + t->stack_size,
                   CLONE_VM | CLONE_FS | CLONE_FILES |
                   CLONE_SIGHAND |CLONE_THREAD | CLONE_SYSVSEM |
                   CLONE_SETTLS |CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID,
                   t,
                   &t->condition,
                   t,
                   &t->condition);
    if(t->tid == -1) {
        deallocate_stack(t->base, t->stack_size);
        free(t);
        return -1;
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
    if(target == NULL) {
        return ESRCH;
    }
    if(target->joined) {
        return EINVAL;
    }

    target->joined = 1;

    int err = futex(&target->condition, FUTEX_WAIT, target->tid);
    if(err == -1 && errno != EAGAIN) {
        return err;
    }

    if(retval) {
        *retval = target->result;
    }
    thread_spin_unlock(&lock);
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