#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "mthread.h"
#include "queue.h"
#include "interrupt.h"
#include "mangle.h"

/* For debugging purposes */
#define DEBUG 0
#define dprintf(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

static queue   *task_q;       /* Queue containing tasks for all threads */
static mthread *current;      /* Thread which is running */
static pid_t unique = 0;   /* To allocate unique Thread IDs */
static mthread_timer_t timer; /* Timer for periodic SIGVTALRM signals */
static size_t stack_size;
static size_t page_size;

/* Tips
 * 1. Use assert()
 * 2. Declare all internal variables and functions (those that are not called
 * by clients of the library) "static" to prevent naming conflicts with programs
 * that link with your thread library.
 */
/* copy a string like strncpy() but always null-terminate */
static char *util_strncpy(char *dst, const char *src, size_t dst_size) {
    if(dst_size == 0)
        return dst;
    
    char *d = dst;
    char *end = dst + dst_size - 1;
    
    while(d < end) {
        if((*d = *src) == '\0')
            return dst;
        d++;
        src++;
    }
    *d = '\0';
    
    return dst;
}

static size_t get_stack_size(void) {
    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    return limit.rlim_cur;
}

static size_t get_page_size() {
    return sysconf(_SC_PAGESIZE);
}

static void * allocate_stack(size_t stack_size) {
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

    return base + page_size;
}

static int deallocate_stack(void *base, size_t stack_size) {
    return munmap(base - page_size, stack_size + page_size);
}

static mthread * get_next_ready_thread(void) {
    mthread *runner;
    int i = getcount(task_q);

    while(i--) {
        runner = dequeue(task_q);

        switch(runner->state) {
            case READY:
                return runner;
            case WAITING:
            case FINISHED:
                enqueue(task_q, runner);
                break;
            case RUNNING:
                return NULL;
        }
    }

    return NULL;
}

static void cleanup_handler(void) {
    dprintf("%-15s: Cleaning up data structures\n", "cleanup_handler");

    mthread *t;
    int n = getcount(task_q);
    while(n--) {
        t = dequeue(task_q);
        deallocate_stack(t->stackaddr, t->stacksize);
        free(t);
    }
    free(task_q);
}

static void thread_start(void) {
    dprintf("%-15s: Entered with TID %d\n", "thread_start", current->tid);
    current->result = current->start_routine(current->arg);
    thread_exit(current->result);
}

static void scheduler(int signum) {
    dprintf("%-15s: SIGVTALRM received\n", "scheduler");
    /* Disable timer interrupts when scheduler is running */
    interrupt_disable(&timer);

    /* Save context and signal masks */
    if(sigsetjmp(current->context, 1) == 1) {
        return;
    }

    dprintf("%-15s: Saving context of Thread TID = %d\n", "scheduler", current->tid);

    /* Change state of current thread from RUNNING to READY*/
    if(current->state == RUNNING)
        current->state = READY;

    /* Enqueue current thread to ready queue */
    enqueue(task_q, current);

    /* Get next ready thread running */
    current = get_next_ready_thread();
    if(current == NULL) {
        exit(0);
    }
    current->state = RUNNING;

    /* Raise all pending signals */
    for(int sig = 1; sig < NSIG; sig++) {
        if(sigismember(&current->sigpending, sig)) {
            raise(sig);
            dprintf("%-15s: Raised pending signal %d of TID = %d\n", "scheduler", sig, current->tid);
        }
    }
    /* Enable timer interrupts before loading next thread */
    interrupt_enable(&timer);

    dprintf("%-15s: Loading context of Thread TID = %d\n", "scheduler", current->tid);
    /* Load context and signal masks */
    siglongjmp(current->context, 1);
}

int thread_init(void) {
    dprintf("%-15s: Started\n", "thread_init");

    /* Initialise queues */
    task_q = calloc(1, sizeof(queue));
    initialize(task_q);

    /* Register cleanup handler to be called on exit */
    atexit(cleanup_handler);

    stack_size = get_stack_size();
    dprintf("%-15s: stack size %lu\n", "thread_init", stack_size);
    page_size = get_page_size();
    dprintf("%-15s: page size %lu\n", "thread_init", page_size);
    
    /* Make thread control block for main thread */
    current = (mthread *) calloc(1, sizeof(mthread));
    current->tid = unique++;
    current->state = RUNNING;
    current->joined_on = -1;

    /* Setting up signal handler */
	// signal(SIGVTALRM, scheduler);
    struct sigaction setup_action;
    sigset_t block_mask;
    sigfillset(&block_mask);
    setup_action.sa_handler = scheduler;
    setup_action.sa_mask = block_mask;
    setup_action.sa_flags = 0;
    sigaction(SIGVTALRM, &setup_action, NULL);

    /* Setup timers for regular interrupts */
    timer.it_value.tv_sec = 0;
	timer.it_interval.tv_sec = 0;
    interrupt_enable(&timer);

    dprintf("%-15s: Exited\n", "thread_init");
    return 0;
}

int thread_create(mthread_t *thread, const mthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    dprintf("%-15s: Started\n", "thread_create");

    interrupt_disable(&timer);
    if(thread == NULL)
        return EFAULT;
    
    if(start_routine == NULL)
        return EFAULT;

    if(unique == MTHREAD_MAX_THREADS) {
        return EAGAIN;
    }

    mthread *tmp = (mthread *) calloc(1, sizeof(mthread));
    if(tmp == NULL) {
        return EAGAIN;
    }

    tmp->tid = unique++;
    tmp->state = READY;
    tmp->start_routine = start_routine;
    tmp->arg = arg;
    tmp->joined_on = -1;
    sigemptyset(&tmp->sigpending);

    /* Stack handling */
    tmp->stacksize = (attr == NULL ? MTHREAD_MIN_STACK : attr->a_stacksize);
    if(tmp->stacksize < MTHREAD_MIN_STACK)
        tmp->stacksize = MTHREAD_MIN_STACK;
    
    tmp->stackaddr = (attr == NULL ? NULL : attr->a_stackaddr);
    if(tmp->stackaddr == NULL) {
        tmp->stackaddr = allocate_stack(tmp->stacksize);
        if(tmp->stackaddr == NULL) {
            return EAGAIN;
        }
    }
    /* Configure remaining attributes */
    if(attr != NULL) {
        /* Overtake fields from the attribute structure */
        tmp->joinable = attr->a_joinable;
        util_strncpy(tmp->name, attr->a_name, MTHREAD_TCB_NAMELEN);
    }
    else {
        /* Set defaults */
        tmp->joinable = 1;
        snprintf(tmp->name, MTHREAD_TCB_NAMELEN, "User%d", tmp->tid);
    }

    /* Make context for new thread */
    sigsetjmp(tmp->context, 1);
    /* Change stack pointer to point to top of stack */
    tmp->context[0].__jmpbuf[JB_SP] = mangle((long int) tmp->stackaddr + tmp->stacksize - sizeof(long int));
    /* Change program counter to point to start function (here thread_start instead) */
	tmp->context[0].__jmpbuf[JB_PC] = mangle((long int) thread_start);

    enqueue(task_q, tmp);
    *thread = tmp->tid;

    interrupt_enable(&timer);
    dprintf("%-15s: Created Thread with TID = %d and put in ready queue\n", "thread_create", tmp->tid);
    return 0;
}

int thread_join(mthread_t tid, void **retval) {
    dprintf("%-15s: Thread TID = %d wants to wait on TID = %d\n", "thread_join", current->tid, tid);
    interrupt_disable(&timer);
    mthread *target;
    target = search_on_tid(task_q, tid);

    /* Deadlock check */
    if(current->tid == tid) {
        return EDEADLK;
    }

    /* Thread exists check */
    if(target == NULL) {
        return ESRCH;
    }

    /* Thread is joinable and no one has joined on it check */
    if(!target->joinable || target->joined_on != -1) {
        return EINVAL;
    }

    target->joined_on = current->tid;
    current->state = WAITING;
    interrupt_enable(&timer);

    while(target->state != FINISHED);

    if(retval) {
        *retval = target->result;
    }

    dprintf("%-15s: Exited\n", "thread_join");
    return 0;
}

/* Exit the calling thread with return value retval. */
void thread_exit(void *retval) {
    dprintf("%-15s: TID %d exiting\n", "thread_exit", current->tid);
    interrupt_disable(&timer);

    current->state = FINISHED;
    current->result = retval;
    
    if(current->joined_on != -1) {
        mthread *target = search_on_tid(task_q, current->joined_on);
        target->state = READY;
    }
    
    interrupt_enable(&timer);
    thread_yield();
}

void thread_yield(void) {
    dprintf("%-15s: Yielding to next thread\n", "thread_yield");
    raise(SIGVTALRM);
}

int thread_kill(mthread_t thread, int sig) {
    dprintf("%-15s: Started\n", "thread_kill");

    if(sig < 0 || sig > NSIG)
        return EINVAL;

    if(thread == current->tid) {
        dprintf("%-15s: Raised signal %d for tid %d\n", "thread_kill", sig, current->tid);
        return raise(sig);
    }

    mthread *target = search_on_tid(task_q, thread);
    if(target == NULL)
        return EINVAL;

    sigaddset(&target->sigpending, sig);

    dprintf("%-15s: Added signal %d to pending signals of TID %d\n", "thread_kill", sig, target->tid);
    dprintf("%-15s: Exited\n", "thread_kill");
    return 0;
}