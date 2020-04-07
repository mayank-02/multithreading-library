#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mthread.h"

// A normal C function that is executed as a thread when its name
// is specified in pthread_create()
void *myThreadFun(void *vargp)
{
	//sleep(1);
	int *max = vargp;
	printf("Max Value = %d\n", *max);
	for(int i = *max; i>=0; i--)
		printf("Printing Threading from Thread \n");
	return NULL;
}

int main()
{
	mthread_t tid;
	int max = 10;
	printf("Before Thread\n");
    thread_init();
	thread_create(&tid, myThreadFun, &max);
	thread_join(tid, NULL);
	sleep(1);
	printf("After Thread\n");
	//exit(0);
    return 0;
}