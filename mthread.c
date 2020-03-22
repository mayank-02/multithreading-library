#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "mthread.h"
#include "queue.h"
#include "interrupt.h"

/* For debugging purposes */
#define DEBUG 0
#define dprintf(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

static queue   *ready_q;     /* Threads which are ready or blocked for another thread */
static queue   *finish_q;    /* Threads which finished but are waiting to be joined */
static mthread *current;     /* Thread which is running */
static uint64_t unique = 0;  /* To allocate unique Thread IDs */
mthread_timer_t timer;      /* Timer for periodic SIGVTALRM signals */

/* Tips
 * 1. Use assert()
 * 2. Declare all internal variables and functions (those that are not called
 * by clients of the library) "static" to prevent naming conflicts with programs
 * that link with your thread library.
 */

#ifdef __x86_64__
	/* Pointer mangling for 64 bit Intel architecture */
	#define JB_SP 6
	#define JB_PC 7
	static long int mangle(long int p) {
		long int ret;
		asm(" mov %1, %%rax;\n"
			" xor %%fs:0x30, %%rax;"
			" rol $0x11, %%rax;"
			" mov %%rax, %0;"
			: "=r"(ret)
			: "r"(p)
			: "%rax"
		);
		return ret;
	}
#else
	/* Pointer mangling for 32 bit Intel architecture */
	typedef unsigned int address_t;
	#define JB_SP 4
	#define JB_PC 5

	address_t mangle(address_t addr) {
		address_t ret  ;
		asm volatile("xor    %%gs:0x18,%0\n"
					"rol    $0x9,%0\n"
					: "=g" (ret)
					: "0" (addr));
		return ret;
	}
#endif


void free_resources(mthread *thread) {
    dprintf("free_resources: Thread with TID %lu freed\n", thread->tid);
    free(thread);
    thread = NULL;
    return;
}

mthread * get_next_ready_thread(void) {
    dprintf("get_next_ready_thread: started\n");

    mthread *runner, *temp;
    while(1) {
        runner = dequeue(ready_q);

        assert(runner->state == READY || runner->state == BLOCKED_JOIN);

        if(runner->state == READY) {
            dprintf("get_next_ready_thread: Returning thread TID = %lu\n", runner->tid);
            return runner;
        }
        else if(runner->state == BLOCKED_JOIN) {
            /* Check if the thread it is waiting for has ended */
            temp = search_on_tid(finish_q, runner->wait_for);

            if(temp == NULL) {
                /* ASSERT: Target thread didn't finish execution */

                /* So put thread back into queue */
                enqueue(ready_q, runner);

                dprintf("get_next_ready_thread: Thread TID = %lu still waiting for TID = %ld\n", runner->tid, runner->wait_for);
            }
            else {
                /* ASSERT: Target thread finished execution */

                /* Free resources of the thread we waited for */
                /* This is WRONG */
                /* free_resources(temp); */

                /* Change state of thread from BLOCKED_JOIN to READY */
                runner->state = READY;
                runner->wait_for = -1;

                dprintf("get_next_ready_thread: Thread TID = %lu wait over\n", runner->tid);
                dprintf("get_next_ready_thread: Returning thread TID = %lu\n", runner->tid);
                return runner;
            }
        }
    }
}

void wrapper(void) {
    dprintf("wrapper: entered\n");
    current->result = current->start_routine(current->arg);
    thread_exit(current->result);
    dprintf("wrapper: exited\n");
}

void scheduler(int signum) {
    dprintf("scheduler: SIGVTALRM received\n");
    /* Disable timer interrupts when scheduler is running */
    interrupt_disable(&timer);
    assert(current->state == RUNNING || current->state == BLOCKED_JOIN);

    /* Save context and signal masks */
    if(sigsetjmp(current->context, 1) == 1) {
        return;
    }

    dprintf("scheduler: Saving context of Thread TID = %lu\n", current->tid);

    /* Change state of current thread from RUNNING to READY*/
    if(current->state == RUNNING)
        current->state = READY;

    /* Enqueue current thread to ready queue */
    enqueue(ready_q, current);

    /* Get next ready thread running*/
    current = get_next_ready_thread();
    current->state = RUNNING;

    /* Enable timer interrupts before loading next thread */
    interrupt_enable(&timer);

    dprintf("scheduler: Loading context of Thread TID = %lu\n", current->tid);
    dprintf("scheduler: current %p\n", current);
    /* Load context and signal masks */
    siglongjmp(current->context, 1);
}

/* Perform any initialization needed. Should be called exactly
 * once, before any other mthread functions.
 */
int thread_init(void) {
    dprintf("thread_init: Started\n");

    /* Initialise queues */
    ready_q = malloc(sizeof(queue));
    initialize(ready_q);
    finish_q = malloc(sizeof(queue));
    initialize(finish_q);

    /* Make thread control block for main thread */
    current = (mthread *) malloc(sizeof(mthread));
    current->tid = unique++;
    current->state = RUNNING;
    current->joinable = 0;
    current->joined_on = -1;
    current->wait_for = -1;
    current->join_arg = NULL;

    /* Setting up signal handler */
	signal(SIGVTALRM, scheduler);

    /* Setup timers for regular interrupts */
    timer.it_value.tv_sec = 0;
	timer.it_interval.tv_sec = 0;
    interrupt_enable(&timer);

    dprintf("thread_init: Exited\n");
    return 0;
}

/* Create a new thread starting at the routine given, which will
 * be passed arg. The new thread does not necessarily execute immediatly
 * (as in, thread_create shouldn't force a switch to the new thread).
 * If the thread will be joined, the joinable flag should be set.
 * Otherwise, it should be 0.
 */
int thread_create(mthread_t *thread, void *(*start_routine)(void *), void *arg) {
    dprintf("thread_create: Started\n");

    interrupt_disable(&timer);
    if(unique == MAX_THREADS) {
        printf("A  system-imposed  limit on the number of threads was encountered.\n");
        return EAGAIN;
    }

    mthread *tmp = (mthread *) malloc(sizeof(mthread));
    if(tmp == NULL) {
        printf("Insufficient resources to create another thread.\n");
        return EAGAIN;
    }
    tmp->tid = unique++;
    tmp->state = READY;
    tmp->start_routine = start_routine;
    tmp->arg = arg;
    tmp->joinable = 1;
    tmp->joined_on = -1;
    tmp->wait_for = -1;

    /* Make context for new thread */
    sigsetjmp(tmp->context, 1);
    /* Change stack pointer to point to top of stack */
    tmp->context[0].__jmpbuf[JB_SP] = mangle((long int) tmp->stack + MIN_STACK);
    /* Change program counter to point to start function (here wrapper instead) */
	tmp->context[0].__jmpbuf[JB_PC] = mangle((long int) wrapper);

    enqueue(ready_q, tmp);
    *thread = tmp->tid;

    interrupt_enable(&timer);
    dprintf("thread_create: Created Thread with TID = %lu and put in ready queue\n", tmp->tid);
    return 0;
}

/* Wait until the specified thread has exited.
 * Returns the value returned by that thread's
 * start function.  Results are undefined if
 * if the thread was not created with the joinable
 * flag set or if it has already been joined.
 */
int thread_join(mthread_t tid, void **retval) {
    dprintf("thread_join: Thread TID = %lu wants to wait on TID = %lu\n", current->tid, tid);
    interrupt_disable(&timer);
    mthread *tmp;
    tmp = search_on_tid(ready_q, tid);
    
    /* Deadlock check */
    if(current->tid == tid) {
        printf("A deadlock was detected.\n");
        return EDEADLK;
    }
    
    /* Thread exists check */
    if(tmp == NULL) {
        tmp = search_on_tid(finish_q, tid);
        if(tmp == NULL) {
            printf("No thread with the ID %lu could be found.\n", tid);
            return ESRCH;
        }
    }

    /* Thread joinable check */
    if(tmp->joinable == 0) {
        printf("%lu is not a joinable thread.\n", tid);
        return EINVAL;
    }

    /* No other thread is waiting on it check */
    if(tmp->joined_on != -1) {
        printf("Another thread is already waiting to join with this thread.\n");
        return EINVAL;
    }

    tmp->joined_on = current->tid;
    current->wait_for = tid;
    current->state = BLOCKED_JOIN;
    interrupt_enable(&timer);
    kill(getpid(), SIGVTALRM);
    /* Wait for target thread to finish
        free target thread resources
        return thread value    
     */
    dprintf("thread_join: Exited\n");
    return 0;
}

/* Exit the calling thread with return value ret. */
void thread_exit(void *retval) {
    interrupt_disable(&timer);
    // dprintf("thread_exit: Thread  TID = %lu is exiting\n", current->tid);
    current->state = FINISHED;
    current->result = retval;

    if(current->tid == 0) {
        /* ASSERT: Main Thread is running */
        if(isempty(ready_q)) {
            exit(0);
        }
        else {
            while(!isempty(ready_q)) {
                mthread *waiting_for;
                waiting_for = get_next_ready_thread();
                thread_join(waiting_for->tid, NULL);
                enqueue(ready_q, waiting_for);

                current->state = BLOCKED_JOIN;

                interrupt_enable(&timer);
                kill(getpid(), SIGVTALRM);
            }
            destroy(ready_q);
            destroy(finish_q);
            free_resources(current);
            dprintf("thread_exit: All threads along with main thread exited\n");
            dprintf("thread_exit: Exiting program now\n");
            exit(0);
        }

    }

    if(current->joinable) {
        enqueue(finish_q, current);
        dprintf("thread_exit: Put Thread TID = %lu in finish queue\n", current->tid);
    }
    else {
        free_resources(current);
    }

    if(isempty(ready_q)) {
        printf("Ready queue is empty and so we say GOODBYE!\n");
        exit(0);
    }
    current = get_next_ready_thread();
    current->state = RUNNING;

    /* Enable timer interrupts before loading next thread */
    interrupt_enable(&timer);
    dprintf("thread_exit: Loading context of Thread TID = %lu\n", current->tid);

    /* Load context and signal masks */
    siglongjmp(current->context, 1);
}

int thread_kill(mthread_t thread, int sig) {
    dprintf("thread_kill: Started\n");
    dprintf("thread_kill: Exited\n");
    return 0;
}