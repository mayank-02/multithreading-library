/**
 * Example code for using spinlocks. This program creates 5 threads
 * each of which tries to increment two variables - one shared and one
 * individual. To access the shared variable spinlocks are used to guarantee
 * mutual exclusion and race conditions. Absence of race conditions are checked
 * by checking if the sum of inidividual values is same as that of the shared
 * variable.
 */

#define _GNU_SOURCE
#include "mthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MCHECK(FCALL)                                                    \
    {                                                                    \
        int result;                                                      \
        if ((result = (FCALL)) != 0) {                                   \
            fprintf(stderr, "FATAL: %s (%s)", strerror(result), #FCALL); \
            exit(-1);                                                    \
        }                                                                \
    }


#define print(str) write(1, str, strlen(str))

long long c = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, running = 1;

mthread_spinlock_t lock;

void *thread1(void *arg) {
	while(running == 1) {
		mthread_spin_lock(&lock);
		c++;
		mthread_spin_unlock(&lock);
		c1++;
	}
}

void *thread2(void *arg) {
	while(running == 1) {
		mthread_spin_lock(&lock);
		c++;
		mthread_spin_unlock(&lock);
		c2++;
	}
}

void *thread3(void *arg) {
	while(running == 1) {
		mthread_spin_lock(&lock);
		c++;
		mthread_spin_unlock(&lock);
		c3++;
	}
}

void *thread4(void *arg) {
	while(running == 1) {
		mthread_spin_lock(&lock);
		c++;
		mthread_spin_unlock(&lock);
		c4++;
	}
}

void *thread5(void *arg) {
	while(running == 1) {
		mthread_spin_lock(&lock);
		c++;
		mthread_spin_unlock(&lock);
		c5++;
	}
}

int main(int argc, char **argv) {
	mthread_t th1, th2, th3, th4, th5;

	if(argc != 2) {
		fprintf(stdout, "Usage: ./program <Time to sleep>\n");
		exit(EXIT_FAILURE);
	}
    fprintf(stdout, "----------------------------------\n");
    fprintf(stdout, "Thread Spinlocks\n");
    fprintf(stdout, "----------------------------------\n");

	mthread_init();
	mthread_spin_init(&lock);

	MCHECK(mthread_create(&th1, NULL, thread1, NULL));
	MCHECK(mthread_create(&th2, NULL, thread2, NULL));
	MCHECK(mthread_create(&th3, NULL, thread3, NULL));
	MCHECK(mthread_create(&th4, NULL, thread4, NULL));
	MCHECK(mthread_create(&th5, NULL, thread5, NULL));
	fprintf(stdout, "Created 5 threads\n");
	fprintf(stdout, "Letting threads run for %d seconds\n", atoi(argv[1]));

	sleep(atoi(argv[1]));

	running = 0;

	MCHECK(mthread_join(th1, NULL));
	MCHECK(mthread_join(th2, NULL));
	MCHECK(mthread_join(th3, NULL));
	MCHECK(mthread_join(th4, NULL));
	MCHECK(mthread_join(th5, NULL));
	fprintf(stdout, "Joined on all 5 threads\n");

	fprintf(stdout, "Thread 1 		= %lld\n", c1);
	fprintf(stdout, "Thread 2 		= %lld\n", c2);
	fprintf(stdout, "Thread 3 		= %lld\n", c3);
	fprintf(stdout, "Thread 4 		= %lld\n", c4);
	fprintf(stdout, "Thread 5 		= %lld\n", c5);
	fprintf(stdout, "t1 + t2 + t3 + t4 + t5  = %lld\n", c1+c2+c3+c4+c5);
	fprintf(stdout, "Shared Variable         = %lld\n", c);
	if(c1+c2+c3+c4+c5 == c) {
		fprintf(stdout, "TEST PASSED\n");
	}
	else{
		fprintf(stdout, "TEST FAILED\n");
	}
	fflush(stdout);
	fprintf(stdout, "----------------------------------\n");
    fprintf(stdout, "Exit Testcases - Thread Spinlocks\n");

   return 0;
}