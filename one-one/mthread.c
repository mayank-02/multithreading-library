#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <linux/futex.h>
#include <sys/time.h>
#include "queue.h"
#include "mthread.h"

static int initialised = 0;
static int nproc;
static rlim_t stack_size;
static queue * task_q;

static rlim_t get_extant_process_limit(void) {
    struct rlimit limit;
    getrlimit(RLIMIT_NPROC, &limit);
    // printf ("The current maximum number of processes is %d.\n", (int) curr_limits.rlim_cur);
    // printf ("The hard limit on the number of processes is %d.\n", (int) curr_limits.rlim_max);
    return limit.rlim_cur;
}

static rlim_t get_stack_size(void) {
    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    // printf("Soft limit: %ju bytes\n", (uintmax_t)limit.rlim_cur);
    // printf("Hard limit: %ju bytes\n", (uintmax_t)limit.rlim_max);
    return limit.rlim_cur;
}

static long get_page_size() {
    long page_size = sysconf(_SC_PAGESIZE);
    if(page_size == -1)
        return NULL;
    // printf("Page Size: %ld\n", page_size);
    return page_size;
}

static void * allocate_stack(rlim_t stack_size) {
    long page_size = get_page_size();
    
    base = mmap(NULL,
                stack_size + page_size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
                -1,
                0);
    if(base == MAP_FAILED)
        return NULL;

    if(mprotect(base, page_size, PROT_READ|PROT_WRITE) == -1) {
        return NULL;
    }

    /* Assume stack grows downward */
    return base + stack_size;
}

static void deallocate_stack(void *base, size_t stack_size) {
    long page_size = get_page_size();
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

/**
 * Obtain information about calling thread. 
 */
static mthread *thread_self(void) {
    uint64_t ptr;
    
    int err = arch_pctl(ARCH_GET_FS, &ptr);
    if(err == -1)
        return NULL;
    
    return (mthread *)ptr;
}

/**
 * Create a new thread starting at the routine given, which will
 * be passed arg.
 */
int thread_create(mthread_t *thread, void *(*start_routine)(void *), void *arg) {
    if(!initialised) {
        task_q = malloc(sizeof(queue));
        initialise(task_q);
        
        mthread *main = (mthread *) malloc(sizeof(mthread));
        main->start_routine = main->arg = main->result = NULL;
        main->tid = gettid();
        enqueue(main);

        stack_size = get_stack_size();
        nproc = get_extant_process_limit();
        initialised = 1;
    }
    
    if(nproc-- == 0) {
        return EAGAIN;
    }
    mthread *t = (mthread *) malloc(sizeof(mthread));
    if(t == NULL) {
        printf("Insufficient resources to create another thread.\n");
        return EAGAIN;
    }
    if(thread == NULL || start_routine == NULL || arg == NULL) {
        return -1;
    }
    t->start_routine = start_routine;
    t->arg = arg;

    t->size = stack_size;
    t->base = allocate_stack(t->size);
    if(t->base == NULL) {
        free(t);
        return -1;
    }

    t->tid = clone( thread_start,
                    t->base + t->size,
                    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
                    CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS |
                    CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID,
                    t,
                    &t->condition,
                    t,
                    &t->condition);
    if(t->tid == -1) {
        deallocate_stack(t->base);
        free(t);
        return -1;
    }

    enqueue(task_q, t);

    *thread = t->tid;

    return 0;
}

void thread_start(void *thread) {
    mthread *t = (mthread *)thread;
    t->result = t->start_routine(t->arg);
    thread_exit(t->result);
}

/**
 * Wait until the specified thread has exited.
 * Returns the value returned by that thread's
 * start function.
 */
int thread_join(mthread_t thread, void **retval) {
    /* Search in queue for tcb corr to tid */
    mthread *target = search_by_tid(task_q, thread);
    if(target == NULL) {
        return ESRCH;
    }
    if(target->joined) {
        return  EINVAL;
    }

    target->joined = 1;

    int err = futex(&target->futex_word, FUTEX_WAIT, target->thread_id);
    if (err == -1) {
        return err;
    }

    if(retval) {
        *retval = target->result;
    }

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
    mthread *self = thread_self();
    thread->result = retval;
    /* WHAT ELSE TO DO HERE? */
    return 0;
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