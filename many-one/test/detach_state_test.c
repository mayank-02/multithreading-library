/**
 * Example of thread creation and termination
 * This is a example to illustrate how execution times are affected
 * by true parallelism versus interleaved execution.
 * The behavior depends on the first argument.
 * See function usage() for details.
 */

#define _XOPEN_SOURCE 500
#define _REENTRANT
#include "mthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <values.h>
#include <errno.h>
#include <time.h>

#define MCHECK(FCALL)                                                    \
    {                                                                    \
        int result;                                                      \
        if ((result = (FCALL)) != 0) {                                   \
            fprintf(stderr, "FATAL: %s (%s)", strerror(result), #FCALL); \
            exit(-1);                                                    \
        }                                                                \
    }

#define NTHREADS 5
#define NITERATIONS 1000000

int idx[NTHREADS];
clock_t start, end;

void *func(void *arg)
{
    int *idx = (int *)arg;
    fprintf(stderr, "Thread %d started\n", *idx);

    /**
     * The following is to keep the CPU busy for a fixed amount
     * of work, so we can compare how long it takes with several
     * threads versus a single thread
     */
    for (long int k = 0; k < NITERATIONS; k++) {
        continue;
    }
    end = clock();

    fprintf(stderr, "Thread %d exited\n", *idx);

    *idx = *idx * 10;
    return (void *)*idx;
}

void usage()
{
    fprintf(stderr, "Usage: detach_state_test [ n | x | j | d ]\n"
                    "n : JOINABLE threads, but don't wait\n"
                    "j : JOINABLE threads, with mthread_join()\n"
                    "d : DETACHED threads, and don't wait\n"
                    "x : DETACHED threads, and wait 3 seconds\n");
}

int main(int argc, char **argv)
{
    int i;
    double elapsed_time = 0;
    mthread_attr_t attrs;
    mthread_t thread[NTHREADS];
    void *value_ptr;

    if (argc != 2) {
        usage();
        exit(-1);
    }
    printf("----------------------------------------------\n");
    printf("Understanding Thread States \n");
    printf("----------------------------------------------\n");

    start = clock();
    for (long int k = 0; k < NITERATIONS; k++) {
        continue;
    }
    end = clock();

    /* Time elapsed in seconds */
    elapsed_time = (end - start) / (double)CLOCKS_PER_SEC;
    fprintf(stderr, "Time taken for one loop = %f seconds\n", elapsed_time);

    mthread_init();
    mthread_attr_init(&attrs);

    /**
     * Use the command-line argument to decide whether to create
     * the threads detached or joinable
     */
    if ((strcmp(argv[1], "d") == 0) || (strcmp(argv[1], "x") == 0)) {
        MCHECK(mthread_attr_set(&attrs, MTHREAD_ATTR_JOINABLE, JOINABLE));
    }

    /* Create threads */
    for (i = 0; i < NTHREADS; i++) {
        idx[i] = i;
        MCHECK(mthread_create(&thread[i], &attrs, func, (void *)&idx[i]));
        fprintf(stderr, "Thread %d created\n", i);
    }


    /* Wait for the threads to complete */
    if (strcmp(argv[1], "j") == 0) {
        for (i = 0; i < NTHREADS; i++) {
            MCHECK(mthread_join(thread[i], &value_ptr));
            fprintf(stderr, "Thread %d returned value %d\n", i, (int)value_ptr);
        }
    }

    /* Find out how long the computation took */
    end = clock();

    if (strcmp(argv[1], "x") == 0) {
        fprintf(stderr, "Main thread waiting 3 seconds\n");
        sleep(3);
    }

    elapsed_time = (end - start) / (double)CLOCKS_PER_SEC;

    fprintf(stderr, "Time taken by all %d threads = %f seconds\n", NTHREADS, elapsed_time);

    fprintf(stderr, "Main thread exiting\n");

    printf("----------------------------------------------\n");

    return 0;
}
