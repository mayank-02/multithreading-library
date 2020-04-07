#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mthread.h"
#include "mutex.h"
#include "cond.h"

#define print(str) write(1, str, strlen(str))

int running = 1, data_ready = 0;
mthread_mutex_t mutex;
mthread_cond_t  condvar;

void *consumer(void *notused) {
    print("In consumer thread...\n");
    while(running) {
        thread_mutex_lock (&mutex);
        while (!data_ready) {
            thread_cond_wait (&condvar, &mutex);
        }
        /* Process data */
        print("Consumer: got data from producer\n");
        data_ready = 0;
        thread_cond_signal (&condvar);
        thread_mutex_unlock (&mutex);
    }
}

void *producer(void *notused) {
    print("In producer thread...\n");
    while(running) {
        /* Simulate getting data from hardware with a sleep (1) */
        sleep (1);
        print("Producer:  got data from hardware\n");
        thread_mutex_lock (&mutex);
        while (data_ready) {
            thread_cond_wait (&condvar, &mutex);
        }
        data_ready = 1;
        thread_cond_signal (&condvar);
        thread_mutex_unlock (&mutex);
    }
}

int main (int argc, char *argv[]) {
    print("Starting consumer/producer example...\n");
    thread_mutex_init(&mutex);
    thread_cond_init(&condvar);
    thread_init();
    mthread_t t1, t2;
    int join;
    mthread_attr_t *attr = mthread_attr_new();
    mthread_attr_get(attr, MTHREAD_ATTR_JOINABLE, &join);
    printf("Join status: Expected 1 Actual %d\n", join);
    mthread_attr_set(attr, MTHREAD_ATTR_JOINABLE, DETACHED);
    mthread_attr_get(attr, MTHREAD_ATTR_JOINABLE, &join);
    printf("Join status: Expected 0 Actual %d\n", join);
    thread_create(&t1, NULL, producer, NULL);
    thread_create(&t2, attr, consumer, NULL);
    mthread_attr_destroy(attr);
    sleep(5);
    running = 0;
    thread_join(t1, NULL);
    if(thread_join(t2, NULL)==-1) {
        
    };
    return 0;
}