/**
 * @file mthread.c
 * @brief Thread control functions
 * @author Mayank Jain
 * @bug No known bugs
 */

#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "mthread.h"
#include "queue.h"
#include "interrupt.h"
#include "mangle.h"
#include "stack.h"
#include "utils.h"

/**
 * @brief For debugging purposes
 */
#ifdef DEBUG
    #define dprintf(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#else
    #define dprintf(fmt, ...) ;
#endif

static queue   *task_q;       ///< Queue containing tasks for all threads
static mthread *current;      ///< Thread which is running
static pid_t unique = 0;      ///< To allocate unique Thread IDs
static mthread_timer_t timer; ///< Timer for periodic SIGVTALRM signals

/**
 * @brief Gets the next ready thread from the task_q
 * @return On success, pointer to the thread; on error, NULL is returned
 */
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

/**
 * @brief Cleans up all malloc(3)ed and mmap(3)ed regions
 */
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

/**
 * @brief Wrapper around user start function
 * @note It makes an implicit call to mthread_exit() to exit the thread
 */
static void mthread_start(void) {
    dprintf("%-15s: Entered with TID %d\n", "mthread_start", current->tid);
    current->result = current->start_routine(current->arg);
    mthread_exit(current->result);
}

/**
 * @brief Schedules next ready thread
 * @param[in] signum Signal number for which this function is a handler
 * @note All signals are blocked in the scheduler
 */
static void scheduler(int signum) {
    dprintf("%-15s: SIGVTALRM received\n", "scheduler");
    /* Disable timer interrupts when scheduler is running */
    interrupt_disable(&timer);

    /* Save context and signal masks */
    if(sigsetjmp(current->context, 1) == 1)
        return;

    dprintf("%-15s: Saving context of Thread TID = %d\n", "scheduler", current->tid);

    /* Change state of current thread from RUNNING to READY*/
    if(current->state == RUNNING)
        current->state = READY;

    /* Enqueue current thread to task queue */
    enqueue(task_q, current);

    /* Get next ready thread running */
    current = get_next_ready_thread();
    if(current == NULL) {
        /* Exit if no threads left to schedule */
        exit(0);
    }
    current->state = RUNNING;

    /* Raise all pending signals */
    for(int sig = 1; sig < NSIG; sig++) {
        if(sigismember(&current->sigpending, sig)) {
            raise(sig);
            sigdelset(&current->sigpending, sig);
            dprintf("%-15s: Raised pending signal %d of TID = %d\n", "scheduler", sig, current->tid);
        }
    }
    /* Enable timer interrupts before loading next thread */
    interrupt_enable(&timer);

    dprintf("%-15s: Loading context of Thread TID = %d\n", "scheduler", current->tid);
    /* Load context and signal masks */
    siglongjmp(current->context, 1);
}

/**
 * @brief Initialise the mthread library
 * @note It has to be the first mthread API function call in an application,
 * and is mandatory.
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_init(void) {
    dprintf("%-15s: Started\n", "mthread_init");

    /* Initialise queues */
    task_q = calloc(1, sizeof(queue));
    initialize(task_q);

    /* Register cleanup handler to be called on exit */
    atexit(cleanup_handler);

    /* Make thread control block for main thread */
    current                = (mthread *) calloc(1, sizeof(mthread));
    current->tid           = unique++;
    current->state         = RUNNING;
    current->joined_on     = -1;
    current->start_routine = current->arg = current->result = NULL;

    /* Setting up signal handler */
    struct sigaction setup_action;
    sigset_t block_mask;
    sigfillset(&block_mask);
    setup_action.sa_handler = scheduler;
    setup_action.sa_mask    = block_mask;
    setup_action.sa_flags   = 0;
    sigaction(SIGVTALRM, &setup_action, NULL);

    /* Setup timer for regular interrupts */
    interrupt_enable(&timer);

    dprintf("%-15s: Exited\n", "mthread_init");
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
int mthread_create(mthread_t *thread, const mthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    dprintf("%-15s: Started\n", "mthread_create");

    if(thread == NULL)
        return EFAULT;

    if(start_routine == NULL)
        return EFAULT;

    if(unique == MTHREAD_MAX_THREADS)
        return EAGAIN;

    mthread *tmp = (mthread *) calloc(1, sizeof(mthread));
    if(tmp == NULL)
        return EAGAIN;

    interrupt_disable(&timer);

    tmp->tid            = unique++;
    tmp->state          = READY;
    tmp->start_routine  = start_routine;
    tmp->arg            = arg;
    tmp->joined_on      = -1;
    sigemptyset(&tmp->sigpending);

    /* Stack handling */
    if(attr != NULL && attr->a_stacksize > MTHREAD_MIN_STACK)
        tmp->stacksize = attr->a_stacksize;
    else
        tmp->stacksize = MTHREAD_MIN_STACK;

    tmp->stackaddr = (attr == NULL ? NULL : attr->a_stackaddr);
    if(tmp->stackaddr == NULL) {
        tmp->stackaddr = allocate_stack(tmp->stacksize);
        if(tmp->stackaddr == NULL) {
            interrupt_enable(&timer);
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
        tmp->joinable = TRUE;
        snprintf(tmp->name, MTHREAD_TCB_NAMELEN, "User%d", tmp->tid);
    }

    /* Make context for new thread */
    sigsetjmp(tmp->context, 1);
    /* Change stack pointer to point to top of stack */
    tmp->context[0].__jmpbuf[JB_SP] = mangle((long int) tmp->stackaddr + tmp->stacksize - sizeof(long int));
    /* Change program counter to point to start function */
	tmp->context[0].__jmpbuf[JB_PC] = mangle((long int) mthread_start);

    enqueue(task_q, tmp);
    *thread = tmp->tid;

    interrupt_enable(&timer);
    dprintf("%-15s: Created Thread with TID = %d and put in task queue\n", "mthread_create", tmp->tid);
    return 0;
}

/**
 * @brief Join with a terminated thread
 * @param[in] tid Handle of thread to wait for
 * @param[in] retval To save the exit status of the target thread
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_join(mthread_t tid, void **retval) {
    dprintf("%-15s: Thread TID = %d wants to wait on TID = %d\n", "mthread_join", current->tid, tid);

    interrupt_disable(&timer);
    mthread *target = search_on_tid(task_q, tid);

    /* Deadlock check */
    if(current->tid == tid) {
        interrupt_enable(&timer);
        return EDEADLK;
    }

    /* Thread exists check */
    if(target == NULL) {
        interrupt_enable(&timer);
        return ESRCH;
    }

    /* Thread is joinable and no one has joined on it check */
    if(!target->joinable || target->joined_on != -1) {
        interrupt_enable(&timer);
        return EINVAL;
    }

    target->joined_on = current->tid;
    current->state = WAITING;
    interrupt_enable(&timer);

    while(target->state != FINISHED);

    if(retval) {
        *retval = target->result;
    }

    dprintf("%-15s: Exited\n", "mthread_join");
    return 0;
}

/**
 * @brief Terminate calling thread
 * @param[in] retval Return value of the thread
 * @return Does not return to the caller
 * @note Performing a return from the start funciton of any thread results in
 * an implicit call to mthread_exit()
 */
void mthread_exit(void *retval) {
    dprintf("%-15s: TID %d exiting\n", "mthread_exit", current->tid);
    interrupt_disable(&timer);

    current->state  = FINISHED;
    current->result = retval;

    if(current->joined_on != -1) {
        mthread *target = search_on_tid(task_q, current->joined_on);
        target->state   = READY;
    }

    interrupt_enable(&timer);
    mthread_yield();
}

/**
 * @brief Yield the processor
 */
void mthread_yield(void) {
    dprintf("%-15s: Yielding to next thread\n", "mthread_yield");
    raise(SIGVTALRM);
}

/**
 * @brief Send a signal to a thread
 * @param[in] thread Thread handle of thread to which signal needs to be sent
 * @param[in] sig    Signal number corresponding to signal
 * @return On success, returns 0; on error, it returns an error number
 */
int mthread_kill(mthread_t thread, int sig) {
    dprintf("%-15s: Started\n", "mthread_kill");
    if(sig < 0 || sig > NSIG)
        return EINVAL;

    if(thread == current->tid) {
        dprintf("%-15s: Raised signal %d for tid %d\n", "mthread_kill", sig, current->tid);
        return raise(sig);
    }

    interrupt_disable(&timer);
    mthread *target = search_on_tid(task_q, thread);
    if(target == NULL) {
        interrupt_enable(&timer);
        return EINVAL;
    }

    sigaddset(&target->sigpending, sig);

    dprintf("%-15s: Added signal %d to pending signals of TID %d\n", "mthread_kill", sig, target->tid);
    dprintf("%-15s: Exited\n", "mthread_kill");
    interrupt_enable(&timer);
    return 0;
}

/**
 * @brief Detach a thread
 * @param[in] thread Thread handle of thread to be detached
 * @return On success, returns 0; on error, it returns an error number
 * @note Once a thread has been detached, it can't be joined with
 * mthread_join() or be made joinable again.
 */
int mthread_detach(mthread_t thread) {
    interrupt_disable(&timer);
    mthread *target = search_on_tid(task_q, thread);

    if(target == NULL) {
        interrupt_enable(&timer);
        return ESRCH;
    }

    if(target->joined_on != -1) {
        interrupt_enable(&timer);
        return EINVAL;
    }

    target->joinable = FALSE;
    dprintf("%-15s: Marked TID %d as detached\n", "mthread_detach", thread);

    interrupt_enable(&timer);
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