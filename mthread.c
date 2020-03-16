#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "mthread.h"
#include "queue.h"

/* For debugging purposes */
#define DEBUG 1
#define dprintf(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

queue *ready_q;     /* Threads which are ready or blocked for another thread */
queue *finish_q;    /* Threads which finished but are waiting to be joined */
mthread_t current;  /* Thread which is running */
static u_int64_t unique = 0; /* To allocate unique Thread IDs */

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

	/* address_t mangle(address_t addr)
	{
		address_t ret;
		asm volatile("xor    %%fs:0x30,%0\n"
					"rol    $0x11,%0\n"
					: "=g" (ret)
					: "0" (addr));
		return ret;
	} */
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

void interrupt_enable(useconds_t timer) {
    dprintf("Timer interrupts enabled for every %ums\n", timer);
    ualarm(timer, timer);
}

void interrupt_disable() {
    dprintf("Timer interrupts disabled\n");
    ualarm(0, 0);
}

void free_resources(mthread_t thread) {
    dprintf("free_resources: Thread with TID %d freed\n", thread->tid);
    free(thread);
    thread = NULL;
    return;
}

mthread_t get_next_ready_thread(void) {
    dprintf("get_next_ready_thread: started\n");

    mthread_t runner, temp;
    while(1) {
        runner = dequeue(ready_q);

        assert(runner->state == READY || runner->state == BLOCKED_JOIN);

        if(runner->state == READY) {
            dprintf("get_next_ready_thread: Returning thread TID = %d\n", runner->tid);
            return runner;
        }
        else if(runner->state == BLOCKED_JOIN) {
            /* Check if the thread it is waiting for has ended */
            temp = search_on_tid(finish_q, runner->wait_on);

            if(temp == NULL) {
                /* ASSERT: Target thread didn't finish execution */

                /* So put thread back into queue */
                enqueue(ready_q, runner);

                dprintf("get_next_ready_thread: Thread TID = %d still waiting for TID = %d\n", runner->tid, runner->wait_on);
            }
            else {
                /* ASSERT: Target thread finished execution */

                /* Free resources of the thread we waited for */
                free_resources(temp);

                /* Change state of thread from BLOCKED_JOIN to READY */
                runner->state = READY;

                dprintf("get_next_ready_thread: Thread TID = %d wait over\n", runner->tid);
                dprintf("get_next_ready_thread: Returning thread TID = %d\n", runner->tid);
                return runner;
            }
        }
    }
}

void wrapper(void) {
    dprintf("wrapper: entered\n");
    interrupt_disable();
    current = get_next_ready_thread();
    current->state = RUNNING;
    interrupt_enable(TIMER);

    current->start_routine(current->arg);
    current->state = FINISHED;

    thread_exit(NULL);
    dprintf("wrapper: exited\n");
}

void scheduler(int signum) {
    dprintf("scheduler: SIGALRM received\n");
    /* Disable timer interrupts when scheduler is running */
    interrupt_disable();

    assert(current->state == RUNNING || current->state == BLOCKED_JOIN);

    dprintf("scheduler: Saving context of Thread TID = %d\n", current->tid);
    /* Save context and signal masks */
    sigsetjmp(current->context, 1);

    /* Change state of current thread from RUNNING to READY*/
    if(current->state == RUNNING)
        current->state = READY;

    /* Enqueue current thread to ready queue */
    enqueue(ready_q, current);

    /* Get next ready thread running*/
    current = get_next_ready_thread();
    current->state = RUNNING;

    /* Enable timer interrupts before loading next thread */
    interrupt_enable(TIMER);

    dprintf("scheduler: Loading context of Thread TID = %d\n", current->tid);
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
    current = (mthread_t) malloc(sizeof(mthread));
    current->tid = unique++;
    current->joinable = 0;

    /* Save context for main thread */
    sigsetjmp(current->context, 1);

    /* Setting up signal handler */
    signal(SIGALRM, scheduler);

    /* Setup timers for regular interrupts */
    interrupt_enable(TIMER);

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

    interrupt_disable();
    mthread_t tmp = (mthread_t) malloc(sizeof(mthread));
    tmp->tid = unique++;
    tmp->state = READY;
    tmp->start_routine = start_routine;
    tmp->arg = arg;
    tmp->joinable = 1;

    /* Make context for new thread */
    sigsetjmp(tmp->context, 1);
    /* Change stack pointer to point to top of stack */
    tmp->context[0].__jmpbuf[JB_SP] = mangle((long int) tmp->stack + MIN_STACK);
    /* Change program counter to point to start function (here wrapper instead) */
	tmp->context[0].__jmpbuf[JB_PC] = mangle((long int) wrapper);

    /* Put thread into ready queue */
    enqueue(ready_q, tmp);

    interrupt_enable(TIMER);
    dprintf("thread_create: Created Thread with TID = %d and put in ready queue\n", tmp->tid);
    return tmp->tid;
}

/* Wait until the specified thread has exited.
 * Returns the value returned by that thread's
 * start function.  Results are undefined if
 * if the thread was not created with the joinable
 * flag set or if it has already been joined.
 */
int thread_join(pid_t tid, void **retval) {
    dprintf("thread_join: Thread TID = %d wants to wait on TID = %d\n", current->tid, tid);
    interrupt_disable();
    current->wait_on = tid;
    current->state = BLOCKED_JOIN;
    interrupt_enable(TIMER);
    kill(getpid(), SIGALRM);
    dprintf("thread_join: Exited\n");
    return 0;
}

/* Exit the calling thread with return value ret. */
void thread_exit(void *retval) {
    dprintf("thread_exit: Thread  TID = %d is exiting\n", current->tid);
    ualarm(0,0);

    if(current->tid == 0) {
        /* ASSERT: Main Thread is running */
        if(isempty(ready_q)) {
            exit(0);
        }
        else {
            while(!isempty(ready_q)) {
                mthread_t waiting_for;
                waiting_for = get_next_ready_thread();
                thread_join(waiting_for->tid, NULL);
                enqueue(ready_q, waiting_for);

                current->state = BLOCKED_JOIN;

                interrupt_enable(TIMER);
                kill(getpid(), SIGALRM);
            }
            destroy(ready_q);
            destroy(finish_q);
            free_resources(current);
            dprintf("thread_exit: All threads along with main thread exited\n");
            dprintf("thread_exit: Exiting program now\n");
            exit(0);
        }

    }
    current->result = retval;

    if(current->joinable) {
        enqueue(finish_q, current);
        dprintf("thread_exit: Put Thread TID = %d in finish queue\n", current->tid);
    }
    else {
        free_resources(current);
    }

    ualarm(TIMER,TIMER);
    dprintf("thread_exit: Exited\n");
    wrapper();
}

/* int thread_lock(mthread_lock_t *lock) {
}

int thread_unlock(mthread_lock_t *lock) {
} */

int thread_kill(mthread_t thread, int sig) {
    dprintf("thread_kill: Started\n");
    dprintf("thread_kill: Exited\n");
    return 0;
}