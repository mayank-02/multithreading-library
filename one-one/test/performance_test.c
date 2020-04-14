/**
 * Example of comparing overhead of thread vs. process creation
 */

#define _XOPEN_SOURCE 500
#define _REENTRANT
#include "mthread.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#define MCHECK(FCALL)                                                     \
    {                                                                    \
        int result;                                                      \
        if ((result = (FCALL)) != 0) {                                   \
            fprintf(stderr, "FATAL: %s (%s)", strerror(result), #FCALL); \
            exit(-1);                                                    \
        }                                                                \
    }

void *thread_body(void *arg) {
    return (void *)NULL;
}

#define US_PER_SEC 1000000

long ticks_per_sec;
long usec_per_tick;
long n_iterations;

clock_t before;
clock_t after;

void check_elapsed_time(clock_t *d) {
    *d = after - before;
    fprintf(stdout, "Number of Iterations = %ld\n", n_iterations);
    fprintf(stdout, "Elapsed Time = %ld ticks\n", *d);
    fprintf(stdout, "Elapsed Time = %ld us\n", *d * usec_per_tick);
    fprintf(stdout, "Avg time per iteration = %ld us\n",
            (*d * usec_per_tick) / n_iterations);
}

int main(int argc, char **argv) {
    int i;
    mthread_t thread;
    void *value_ptr;
    pid_t child;
    int status;
    clock_t d1, d2;
    mthread_init();

    fprintf(stdout, "------------------------------\n");
    fprintf(stdout, "System Clock Ticks Per Sec = %ld\n",
            (long)CLOCKS_PER_SEC);
    usec_per_tick = US_PER_SEC / CLOCKS_PER_SEC;
    fprintf(stdout, "Microseconds Per Clock Tick = %ld\n", usec_per_tick);

    fprintf(stdout, "----------Processes-----------\n");

    n_iterations = 1000;

    before = clock();
    for (i = 1; i < n_iterations; i++) {
        if ((child = fork())) { /* we are the parent */
            waitpid(child, &status, 0);
        }
        else { /* we are the child */
            exit(0);
        }
    }
    after = clock();

    check_elapsed_time(&d1);

    fprintf(stdout, "-----------Threads------------\n");

    n_iterations = 1000;

    before = clock();
    for (i = 1; i < n_iterations; i++) {
        /* Use the default attributes */
        MCHECK(mthread_create(
            &thread,     /* place to store the id of new thread */
            MTHREAD_ATTR_DEFAULT,      /* thread creation attributes */
            thread_body, /* function for thread to execute */
            NULL));       /* pass no parameter */
        MCHECK(mthread_join(thread, &value_ptr));
    }
    after = clock();

    check_elapsed_time(&d2);

    fprintf(stdout, "------------------------------\n");
    if (d2 > 0.0)
        fprintf(stdout, "Performance Ratio = %f\n", (double)d1/d2);

    exit(0);
}
