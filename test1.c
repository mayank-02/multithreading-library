#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mthread.h"

/* Number of threads to use */
#define NUM_OF_THREADS 3

void *print_message_function(void *ptr) {
     printf("%s\n", (char *)ptr);
     return NULL;
}

int main() {
     /* Thread identifiers */
     mthread_t tid[NUM_OF_THREADS];
     char *message1 = "Thread 1";
     char *message2 = "Thread 2";
     int  iret1, iret2;

     thread_init();

     /* Create independent threads each of which will execute function */
     iret1 = thread_create(&tid[0], print_message_function, (void *) message1);
     iret2 = thread_create(&tid[1], print_message_function, (void *) message2);

     sleep(5);
     /* Wait until all threads complete */
     for(int i = 0; i < NUM_OF_THREADS; i++) {
          thread_join(tid[i], NULL);
     }
     printf("Thread 1 returns: %d\n",iret1);
     printf("Thread 2 returns: %d\n",iret2);
     thread_exit(NULL);
     return 0;
}

