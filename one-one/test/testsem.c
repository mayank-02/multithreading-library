// C program to demonstrate working of Semaphores 
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <unistd.h> 
#include "mthread.h" 
#include "sem.h" 
  
#define print(str) write(1, str, strlen(str))

typedef int buffer_item;
#define BUFFER_SIZE 5

#define RAND_DIVISOR 1000000000
#define TRUE 1

/* The mutex lock */
mthread_mutex_t mutex;

/* the semaphores */
mthread_sem_t full, empty;

/* the buffer */
buffer_item buffer[BUFFER_SIZE];

/* buffer counter */
int counter;

mthread_t tid;       //Thread ID

void *producer(void *param); /* the producer thread */
void *consumer(void *param); /* the consumer thread */

void initializeData() {

   /* Create the mutex lock */
   thread_mutex_init(&mutex);

   /* Create the full semaphore and initialize to 0 */
   thread_sem_init(&full, 0);

   /* Create the empty semaphore and initialize to BUFFER_SIZE */
   thread_sem_init(&empty, BUFFER_SIZE);

    /* Initialise the library */
    thread_init();
   /* init buffer */
   counter = 0;
}


/* Add an item to the buffer */
int insert_item(buffer_item item) {
   /* When the buffer is not full add the item
      and increment the counter*/
   if(counter < BUFFER_SIZE) {
      buffer[counter] = item;
      counter++;
      return 0;
   }
   else { /* Error the buffer is full */
      return -1;
   }
}

/* Remove an item from the buffer */
int remove_item(buffer_item *item) {
   /* When the buffer is not empty remove the item
      and decrement the counter */
   if(counter > 0) {
      *item = buffer[(counter-1)];
      counter--;
      return 0;
   }
   else { /* Error buffer empty */
      return -1;
   }
}


/* Producer Thread */
void *producer(void *param) {
   buffer_item item;

   while(TRUE) {
      /* sleep for a random period of time */
      long rNum = rand() / RAND_DIVISOR;
      fprintf(stderr, "Producer sleeping for %ld seconds\n", rNum);
      sleep(rNum);

      /* generate a random number */
      item = rand();

      /* acquire the empty lock */
      thread_sem_wait(&empty);
      /* acquire the mutex lock */
      thread_mutex_lock(&mutex);

      if(insert_item(item)) {
         fprintf(stderr, " Producer report error condition\n");
      }
      else {
         printf("producer produced %d\n", item);
      }
      /* release the mutex lock */
      thread_mutex_unlock(&mutex);
      /* signal full */
      thread_sem_post(&full);
   }
}

/* Consumer Thread */
void *consumer(void *param) {
   buffer_item item;

   while(TRUE) {
      /* sleep for a random period of time */
      long rNum = rand() / RAND_DIVISOR;
      fprintf(stderr, "Consumer sleeping for %ld seconds\n", rNum);
      sleep(rNum);

      /* aquire the full lock */
      thread_sem_wait(&full);
      /* aquire the mutex lock */
      thread_mutex_lock(&mutex);
      if(remove_item(&item)) {
         fprintf(stderr, "Consumer report error condition\n");
      }
      else {
         printf("consumer consumed %d\n", item);
      }
      /* release the mutex lock */
      thread_mutex_unlock(&mutex);
      /* signal empty */
      thread_sem_post(&empty);
   }
}

int main(int argc, char *argv[]) {
   /* Loop counter */
   int i;

   /* Verify the correct number of arguments were passed in */
   if(argc != 4) {
      fprintf(stderr, "USAGE:./main.out <INT> <INT> <INT>\n");
   }

   int mainSleepTime = atoi(argv[1]); /* Time in seconds for main to sleep */
   int numProd = atoi(argv[2]); /* Number of producer threads */
   int numCons = atoi(argv[3]); /* Number of consumer threads */

   /* Initialize the app */
   initializeData();

   /* Create the producer threads */
   for(i = 0; i < numProd; i++) {
      /* Create the thread */
      thread_create(&tid, NULL, producer, NULL);
    }

   /* Create the consumer threads */
   for(i = 0; i < numCons; i++) {
      /* Create the thread */
      thread_create(&tid, NULL, consumer, NULL);
   }

   /* Sleep for the specified amount of time in milliseconds */
   sleep(mainSleepTime);

   /* Exit the program */
   printf("Exit the program\n");
   exit(0);
}