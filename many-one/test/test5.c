#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mthread.h"

/* Number of threads to use */
#define NUM_OF_THREADS 3

void *print_msg(void *ptr) {
     sleep(0.2);
     printf("%s\n", (char *)ptr);
     thread_exit(NULL);
}

int main() {
     /* Thread identifiers */
     mthread_t tid[NUM_OF_THREADS];
     char *message1 = "Thread 1";
     char *message2 = "Thread 2";
     char *message3 = "Thread 3";
     int  iret1, iret2, iret3, join = 2;

     thread_init();
     mthread_attr_t *attr = mthread_attr_new();
     mthread_attr_get(attr, MTHREAD_ATTR_JOINABLE, &join);
     printf("Join status: Expected 1 Actual %d\n", join);
     mthread_attr_set(attr, MTHREAD_ATTR_JOINABLE, 0);
     mthread_attr_get(attr, MTHREAD_ATTR_JOINABLE, &join);
     printf("Join status: Expected 0 Actual %d\n", join);
     
     /* Create independent threads each of which will execute function */
     iret1 = thread_create(&tid[0], attr, print_msg, (void *) message1);
     if(iret1 == -1) {
          perror("thread_create1");
          exit(-1);
     }
     iret2 = thread_create(&tid[1], NULL, print_msg, (void *) message2);
     if(iret2 == -1) {
          perror("thread_create2");
          exit(-1);
     }
     iret3 = thread_create(&tid[2], NULL, print_msg, (void *) message3);
     if(iret3 == -1) {
          perror("thread_create3");
          exit(-1);
     }

     /* Wait until all threads complete */
     for(int i = 0; i < NUM_OF_THREADS - 1; i++) {
          thread_join(tid[i], NULL);
     }

     printf("Thread 1 returns: %d\n",iret1);
     printf("Thread 2 returns: %d\n",iret2);
     printf("Thread 3 returns: %d\n",iret3);
     
     mthread_attr_destroy(attr);
     thread_exit(NULL);
     return 0;
}

