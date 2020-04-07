#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include "mthread.h"
#include "mutex.h"

#define print(str) write(1, str, strlen(str))

long long c = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, run = 1;
mthread_mutex_t flag;

void *thread1(void *arg) {
	while(run == 1) {
		thread_mutex_lock(&flag);
      // print("Thread1: locked\n");
		c++;
      // print("Thread1: unlocked\n");
		thread_mutex_unlock(&flag);
		c1++;
	}
}

void *thread2(void *arg) {
	while(run == 1) {
		thread_mutex_lock(&flag);
      // print("Thread2: locked\n");
		c++;
      // print("Thread2: unlocked\n");
		thread_mutex_unlock(&flag);
		c2++;
	}
}

void *thread3(void *arg) {
	while(run == 1) {
		thread_mutex_lock(&flag);
      // print("Thread3: locked\n");
		c++;
      // print("Thread3: unlocked\n");
		thread_mutex_unlock(&flag);
		c3++;
	}
}

void *thread4(void *arg) {
	while(run == 1) {
		thread_mutex_lock(&flag);
      // print("Thread4: locked\n");
		c++;
      // print("Thread4: unlocked\n");
		thread_mutex_unlock(&flag);
		c4++;
	}
}

void *thread5(void *arg) {
	while(run == 1) {
		thread_mutex_lock(&flag);
      // print("Thread5: locked\n");
		c++;
      // print("Thread5: unlocked\n");
		thread_mutex_unlock(&flag);
		c5++;
	}
}

int main() {
	mthread_t th1, th2, th3, th4, th5;

	thread_mutex_init(&flag);
	thread_init();
	thread_create(&th1, NULL, thread1, NULL);
	thread_create(&th2, NULL, thread2, NULL);
	thread_create(&th3, NULL, thread3, NULL);
	thread_create(&th4, NULL, thread4, NULL);
	thread_create(&th5, NULL, thread5, NULL);

	sleep(2);
	run = 0;

	thread_join(th1, NULL);
	thread_join(th2, NULL);
	thread_join(th3, NULL);
	thread_join(th4, NULL);
	thread_join(th5, NULL);

	fprintf(stdout, "c              = %lld \n", c);
	fprintf(stdout, "c1+c2+c3+c4+c5 = %lld \n", c1+c2+c3+c4+c5);
	fprintf(stdout, "c1 = %lld c2 = %lld c3 = %lld c4 = %lld c5 = %lld\n", c1, c2, c3, c4, c5);
	fflush(stdout);

   return 0;
}