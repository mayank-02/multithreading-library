/**
 * Example code for using mthread condition variables.  The main thread
 * creates three threads.  Two of those threads increment a "count" variable,
 * while the third thread watches the value of "count".  When "count"
 * reaches a predefined limit, the waiting thread is signaled by one of the
 * incrementing threads. The waiting thread "awakens" and then modifies
 * count. The program continues until the incrementing threads reach
 * TCOUNT. The main program prints the final value of count.
 */

#include "mthread.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MCHECK(FCALL)                                                     \
    {                                                                    \
        int result;                                                      \
        if ((result = (FCALL)) != 0) {                                   \
            fprintf(stderr, "FATAL: %s (%s)", strerror(result), #FCALL); \
            exit(-1);                                                    \
        }                                                                \
    }

#define NUM_THREADS 3
#define TCOUNT 10
#define COUNT_LIMIT 12

int count = 0;
mthread_mutex_t count_mutex;
mthread_cond_t count_threshold_cv;

void *inc_count(void *t) {
    int i;
    long my_id = (long)t;

    for (i = 0; i < TCOUNT; i++) {
        mthread_mutex_lock(&count_mutex);
        count++;

        /*
         * Check the value of count and signal waiting thread when condition is
         * reached. Note that this occurs while mutex is locked.
         */

        if (count == COUNT_LIMIT) {
            printf("inc_count()  : thread %ld, count = %d, threshold reached. ",
                   my_id, count);
            mthread_cond_signal(&count_threshold_cv);
            printf("Just sent signal.\n");
        }
        printf("inc_count()  : thread %ld, count = %d, unlocking mutex\n",
               my_id, count);
        mthread_mutex_unlock(&count_mutex);

        /* Do some work so threads can alternate on mutex lock */
        sleep(1);
    }
    mthread_exit(NULL);
}

void *watch_count(void *t) {
    long my_id = (long)t;

    printf("Starting watch_count(): thread %ld\n", my_id);

    /**
     * Lock mutex and wait for signal.  Note that the mthread_cond_wait routine
     * will automatically and atomically unlock mutex while it waits.
     * Also, note that if COUNT_LIMIT is reached before this routine is run by
     * the waiting thread, the loop will be skipped to prevent mthread_cond_wait
     * from never returning.
    */
    mthread_mutex_lock(&count_mutex);

    while (count < COUNT_LIMIT) {
        printf("watch_count(): thread %ld, count = %d, Going into wait...\n", my_id, count);
        mthread_cond_wait(&count_threshold_cv, &count_mutex);
        printf("watch_count(): thread %ld, count = %d, Condition signal received\n",  my_id, count);
    }
    printf("watch_count(): thread %ld, updating the value of count...\n", my_id);
    count += 125;
    printf("watch_count(): thread %ld, count now = %d\n", my_id, count);
    printf("watch_count(): thread %ld, unlocking mutex\n", my_id);
    mthread_mutex_unlock(&count_mutex);
    mthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int i;
    long t1 = 1, t2 = 2, t3 = 3;
    mthread_t threads[3];

    printf("-------------------------------------------\n");
    printf("Thread Condition Variable\n");
    printf("-------------------------------------------\n");

    /* Initialize library, mutex and condition variable objects */
    mthread_init();
    printf("Thread Library Initialised\n");

    mthread_mutex_init(&count_mutex);
    printf("Mutex Initialised\n");

    mthread_cond_init(&count_threshold_cv);
    printf("Condition Variable Initialised\n");


    MCHECK(mthread_create(&threads[0], NULL, watch_count, (void *)t1));
    MCHECK(mthread_create(&threads[1], NULL, inc_count, (void *)t2));
    MCHECK(mthread_create(&threads[2], NULL, inc_count, (void *)t3));

    /* Wait for all threads to complete */
    for (i = 0; i < NUM_THREADS; i++) {
        MCHECK(mthread_join(threads[i], NULL));
    }

    printf("Main(): Waited and joined with %d threads.\n", NUM_THREADS);
    printf("Final value of count = %d.\n", count);

    if(count == 145) {
        printf("TEST PASSED\n");
    }
    else {
        printf("TEST FAILED\n");
    }
    printf("Exit  Testcases - Thread Condition Variable\n");
    return 0;
}