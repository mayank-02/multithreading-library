#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "mthread.h"
#include "spin_lock.h"

#define print(str) write(STDOUT_FILENO, str, strlen(str))

mthread_spinlock_t value;
int hola;

void handler(int sig) {
    print("Inside the handler\n");
    thread_exit((void *)128);
}

void *func1(void *arg) {
    print("Thread1: Entered\n");
    signal(SIGUSR1, handler);

    thread_spin_lock(&value);
    print("Thread1: Locked\n");    
    
    print("Thread1: Yielding\n");
    thread_yield();
    print("Thread1: Came back after yielding\n");
    
    hola += 5;
    
    sleep(2);
    print("Thread1: Slept for 2s\n");
    
    print("Thread1: Unlocked\n");
    thread_spin_unlock(&value);
    
    print("Thread1: Exited\n");

    return (void *)hola;
}

void *func2(void *arg) {
    print("Thread2: Entered\n");
    
    thread_spin_lock(&value);
    print("Thread2: Locked\n");
    hola = 15;
    print("Thread2: Unlocked\n");
    thread_spin_unlock(&value);
    
    print("Thread2: Exited\n");
    thread_exit((void *)hola);
}

int main() {
    mthread_t td1, td2;
    void *ret1, *ret2;
    int err;
    
    thread_spin_init(&value);
    
    err = thread_create(&td2, func1, NULL);
    if(err == -1) {
        perror("thread_create1");
    }
    
    err = thread_create(&td1, func2, NULL);
    if(err == -1) {
        perror("thread_create2");
    }

    sleep(1);

    thread_kill(td1, SIGUSR1);

    print("In main\n");
    err = thread_join(td1, &ret1);
    if(err == -1) {
        perror("thread_join1");
    }
    printf("Thread1: Returned %d\n", (int)ret1);
    
    err = thread_join(td2, &ret2);
    if(err == -1) {
        perror("thread_join2");
    }
    printf("Thread2: Returned %d\n", (int)ret2);

    return 0;
}
