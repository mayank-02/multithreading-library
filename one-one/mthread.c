#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include "mthread.h"

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

/**
 * Create a new thread starting at the routine given, which will
 * be passed arg.
 */
int thread_create(mthread_t *thread, void *(*start_routine)(void *), void *arg) {
    mthread *t = (mthread *) malloc(sizeof(mthread));
    if(t == NULL) {
        printf("Insufficient resources to create another thread.\n");
        return EAGAIN;
    }

    t->start_routine = start_routine;
    t->arg = arg;

    t->size = get_stack_size();
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
                    thread_start,
                    /* Add more args */
                    );
    if(t->tid == -1) {
        /* Error checks */
        deallocate_stack(t->base);
        free(t);
        return -1;
    }

    *thread = t->tid;
    return 0;
}

void thread_start(void) {
}

/**
 * Wait until the specified thread has exited.
 * Returns the value returned by that thread's
 * start function.
 */
int thread_join(mthread_t thread, void **retval) {
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
}

/**
 * Send signal specified by sig to thread
 */
int thread_kill(mthread_t thread, int sig) {
}