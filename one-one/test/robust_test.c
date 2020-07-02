/**
 * Unit testing for public API functions provided by the library
 */

#define _REENTRANT
#include "mthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define NTHREADS 5

#define print(str)  write(STDOUT_FILENO, str, strlen(str))

#define MCHECKPASS(FCALL)                                                \
    {                                                                    \
        int result;                                                      \
        if ((result = (FCALL)) != 0) {                                   \
            fprintf(stderr, "FATAL: %s (%s)\n", strerror(result), #FCALL); \
            fprintf(stdout, "TEST FAILED\n\n");                            \
            exit(EXIT_FAILURE);                            \
        }                                                                \
        else {                                                           \
            fprintf(stdout, "TEST PASSED\n\n");                            \
        }                                                                \
    }

#define MCHECKFAIL(FCALL)                                                \
    {                                                                    \
        int result;                                                      \
        if ((result = (FCALL)) != 0) {                                   \
            fprintf(stderr, "FATAL: %s (%s)\n", strerror(result), #FCALL); \
            fprintf(stdout, "TEST PASSED\n\n");                            \
        }                                                                \
        else {                                                           \
            fprintf(stdout, "TEST FAILED\n\n");                            \
            exit(EXIT_FAILURE);                            \
        }                                                                \
    }

#define MCHECK(FCALL)                                                    \
    {                                                                    \
        int result;                                                      \
        if ((result = (FCALL)) != 0) {                                   \
            fprintf(stderr, "FATAL: %s (%s)\n", strerror(result), #FCALL); \
        }                                                                \
    }

int running;

void handler(int sig) {
    print("Inside the handler\n");
    running = 0;
}

void *thread1(void *arg) {
    mthread_exit((void *)64);
}

void *thread2(void *arg) {
    return (void *)2;
}

void *infinite(void *arg) {
    while(running);
    return (void *)128;
}

int main(int argc, char **argv) {
    mthread_init();

    printf("-------------------------------------------\n");
    printf("Thread Attribute Handling\n");
    printf("-------------------------------------------\n");
    printf("1] Handling detach state of a thread\n");
    {
        int join;
        mthread_attr_t *attr = mthread_attr_new();
        MCHECK(mthread_attr_get(attr, MTHREAD_ATTR_JOINABLE, &join));
        printf("Detach State: Expected 1 Actual %d\n", join);
        MCHECK(mthread_attr_set(attr, MTHREAD_ATTR_JOINABLE, DETACHED));
        MCHECK(mthread_attr_get(attr, MTHREAD_ATTR_JOINABLE, &join));
        printf("Detach State: Expected 0 Actual %d\n", join);
        MCHECK(mthread_attr_destroy(attr));
        printf("TEST PASSED\n\n");
    }

    printf("2] Handling stack of a thread\n");
    {
        size_t s;
        void  *t;
        void *stack = malloc(10 * 1024);
        mthread_attr_t *attr = mthread_attr_new();
        MCHECK(mthread_attr_get(attr, MTHREAD_ATTR_STACK_SIZE, &s));
        MCHECK(mthread_attr_get(attr, MTHREAD_ATTR_STACK_ADDR, &t));
        printf("Default stack size    = %lu\n", s);
        printf("Default stack address = %p\n", t);

        MCHECK(mthread_attr_set(attr, MTHREAD_ATTR_STACK_SIZE, 10 * 1024));
        MCHECK(mthread_attr_set(attr, MTHREAD_ATTR_STACK_ADDR, stack));
        printf("Set stack size        = 10240\n");
        printf("Set stack address     = %p\n", stack);

        MCHECK(mthread_attr_get(attr, MTHREAD_ATTR_STACK_SIZE, &s));
        MCHECK(mthread_attr_get(attr, MTHREAD_ATTR_STACK_ADDR, &t));
        printf("New stack size        = %lu\n", s);
        printf("New stack address     = %p\n", t);

        MCHECK(mthread_attr_destroy(attr));
        if(s == 10240 && t == stack) {
            printf("TEST PASSED\n\n");
        }
        else {
            printf("TEST FAILED\n\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("-------------------------------------------\n");
    printf("Thread Create\n");
    printf("-------------------------------------------\n");
    printf("1] Start Routine is NULL\n");
    {
        mthread_t tid;
        MCHECKFAIL(mthread_create(&tid, NULL, NULL, NULL));
    }

    printf("2] Create Thread with Default Attributes\n");
    {
        mthread_t tid;
        MCHECKPASS(mthread_create(&tid, NULL, thread1, NULL));
        MCHECK(mthread_join(tid, NULL));
    }

    printf("3] Create Detached Thread\n");
    {
        mthread_t tid;
        mthread_attr_t attr;
        mthread_attr_init(&attr);
        mthread_attr_set(&attr, MTHREAD_ATTR_JOINABLE, DETACHED);
        MCHECK(mthread_create(&tid, &attr, thread1, NULL));
        MCHECKFAIL(mthread_join(tid, NULL));
    }

    printf("4] Create Thread having Stack Size of 10KB\n");
    {
        mthread_t tid;
        mthread_attr_t attr;
        mthread_attr_init(&attr);
        mthread_attr_set(&attr, MTHREAD_ATTR_STACK_SIZE, 10 * 1024);
        void *stack = malloc(10 * 1024);
        mthread_attr_set(&attr, MTHREAD_ATTR_STACK_ADDR, stack);
        MCHECKPASS(mthread_create(&tid, &attr, thread1, NULL));
        MCHECK(mthread_join(tid, NULL));
    }

    printf("-------------------------------------------\n");
    printf("Thread Join\n");
    printf("-------------------------------------------\n");
    printf("1] Invalid Thread Handle Passed\n");
    {
        mthread_t tid;
        MCHECK(mthread_create(&tid, NULL, thread1, NULL));
        MCHECKFAIL(mthread_join(1000, NULL));
    }

    printf("2] Joining on a detached thread\n");
    {
        mthread_t tid;
        mthread_attr_t attr;
        mthread_attr_init(&attr);
        mthread_attr_set(&attr, MTHREAD_ATTR_JOINABLE, DETACHED);
        MCHECK(mthread_create(&tid, &attr, thread1, NULL));
        MCHECKFAIL(mthread_join(tid, NULL));
    }

    printf("3] Joining on an already joined thread\n");
    {
        mthread_t tid;
        MCHECK(mthread_create(&tid, NULL, thread1, NULL));
        MCHECK(mthread_join(tid, NULL));
        MCHECKFAIL(mthread_join(tid, NULL));
    }

    printf("4] Joining on a thread and collecting exit status\n");
    {
        void *value;
        mthread_t tid;
        MCHECK(mthread_create(&tid, NULL, thread1, NULL));
        MCHECK(mthread_join(tid, &value));
        fprintf(stdout, "Expected Exit Status is %d\n", 64);
        fprintf(stdout, "Actual   Exit Status is %d\n", (int) value);
        if((int)value == 64) {
            fprintf(stdout, "TEST PASSED\n\n");
        }
        else {
            fprintf(stdout, "TEST FAILED\n\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("5] Joining on more than one thread\n");
    {
        mthread_t tid[5];
        for(int i = 0; i < 5; i++) {
            MCHECK(mthread_create(&tid[i], NULL, thread1, NULL));
            fprintf(stdout, "Thread %d created\n", i);
        }
        for(int i = 0; i < 5; i++) {
            MCHECK(mthread_join(tid[i], NULL));
            fprintf(stdout, "Thread %d joined\n", i);
        }
        fprintf(stdout, "TEST PASSED\n\n");
    }

    printf("-------------------------------------------\n");
    printf("Thread Kill\n");
    printf("-------------------------------------------\n");
    printf("1] Send invalid signal\n");
    {
        mthread_t tid;
        signal(SIGUSR1, handler);
        running = 1;
        MCHECK(mthread_create(&tid, NULL, infinite, NULL));
        MCHECKFAIL(mthread_kill(tid, -1));
        running = 0;
        MCHECK(mthread_join(tid, NULL));
    }

    printf("2] Send signal to a thread\n");
    {
        void *value;
        mthread_t tid;
        signal(SIGUSR1, handler);
        running = 1;
        MCHECK(mthread_create(&tid, NULL, infinite, NULL));
        MCHECK(mthread_kill(tid, SIGUSR1));
        MCHECK(mthread_join(tid, &value));
        if((int)value == 128) {
            fprintf(stdout, "TEST PASSED\n\n");
        }
        else {
            fprintf(stdout, "TEST FAILED\n\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("-------------------------------------------\n");
    printf("Thread Exit\n");
    printf("-------------------------------------------\n");
    printf("1] Created Thread Uses Return To Exit\n");
    {
        void *value;
        mthread_t tid;

        MCHECK(mthread_create(&tid, NULL, thread1, NULL));
        MCHECK(mthread_join(tid, &value));
        fprintf(stdout, "Expected Exit Status is %d\n", 64);
        fprintf(stdout, "Actual   Exit Status is %d\n", (int) value);
        if((int)value == 64) {
            fprintf(stdout, "TEST PASSED\n\n");
        }
        else {
            fprintf(stdout, "TEST FAILED\n\n");
            exit(EXIT_FAILURE);
        }

    }
    printf("2] Created Thread Uses mthread_exit()\n");
    {
        void *value;
        mthread_t tid;

        MCHECK(mthread_create(&tid, NULL, thread2, NULL));
        MCHECK(mthread_join(tid, &value));
        fprintf(stdout, "Expected Exit Status is %d\n", 2);
        fprintf(stdout, "Actual   Exit Status is %d\n", (int) value);
        if((int)value == 2) {
            fprintf(stdout, "TEST PASSED\n\n");
        }
        else {
            fprintf(stdout, "TEST FAILED\n\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}