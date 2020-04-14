/**
 * Example to demonstrate working of semaphores by solving
 * the producer consumer problem. Producers generates random
 * numbers which are put in the buffer and the consumers consume them
 * without having a race on the buffer.
 */
#include "mthread.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MCHECK(FCALL)                                                    \
    {                                                                    \
        int result;                                                      \
        if ((result = (FCALL)) != 0)                                     \
        {                                                                \
            fprintf(stderr, "FATAL: %s (%s)", strerror(result), #FCALL); \
            exit(-1);                                                    \
        }                                                                \
    }

#define print(str) write(1, str, strlen(str))

#define TRUE 1
#define BUFFER_SIZE 5
#define RAND_DIVISOR 1000000000

typedef int buffer_item;

/* The mutex lock */
mthread_mutex_t mutex;

/* the semaphores */
mthread_sem_t full, empty;

/* the buffer */
buffer_item buffer[BUFFER_SIZE];

/* buffer counter */
int counter;

/* Thread ID */
mthread_t tid;

/* the producer thread */
void *producer(void *param);

/* the consumer thread */
void *consumer(void *param);

void initializeData()
{
    /* Create the mutex lock */
    mthread_mutex_init(&mutex);

    /* Create the full semaphore and initialize to 0 */
    mthread_sem_init(&full, 0);

    /* Create the empty semaphore and initialize to BUFFER_SIZE */
    mthread_sem_init(&empty, BUFFER_SIZE);

    /* Initialise the library */
    mthread_init();

    /* init buffer */
    counter = 0;
}

/* Add an item to the buffer */
int insert_item(buffer_item item)
{
    /* When the buffer is not full add the item
      and increment the counter*/
    if (counter < BUFFER_SIZE)
    {
        buffer[counter] = item;
        counter++;
        return 0;
    }
    else
    { /* Error the buffer is full */
        return -1;
    }
}

/* Remove an item from the buffer */
int remove_item(buffer_item *item)
{
    /* When the buffer is not empty remove the item
      and decrement the counter */
    if (counter > 0)
    {
        *item = buffer[(counter - 1)];
        counter--;
        return 0;
    }
    else
    { /* Error buffer empty */
        return -1;
    }
}

/* Producer Thread */
void *producer(void *param)
{
    buffer_item item;

    while (TRUE)
    {
        /* sleep for a random period of time */
        long rNum = rand() / RAND_DIVISOR;
        // fprintf(stderr, "Producer sleeping for %ld seconds\n", rNum);
        sleep(rNum);

        /* generate a random number */
        item = rand();

        /* acquire the empty lock */
        mthread_sem_wait(&empty);
        /* acquire the mutex lock */
        mthread_mutex_lock(&mutex);

        if (insert_item(item))
        {
            fprintf(stderr, " Producer report error condition\n");
        }
        else
        {
            printf("Producer produced %d\n", item);
        }
        /* release the mutex lock */
        mthread_mutex_unlock(&mutex);
        /* signal full */
        mthread_sem_post(&full);
    }
}

/* Consumer Thread */
void *consumer(void *param)
{
    buffer_item item;

    while (TRUE)
    {
        /* sleep for a random period of time */
        long rNum = rand() / RAND_DIVISOR;
        // fprintf(stderr, "Consumer sleeping for %ld seconds\n", rNum);
        sleep(rNum);

        /* aquire the full lock */
        mthread_sem_wait(&full);
        /* aquire the mutex lock */
        mthread_mutex_lock(&mutex);
        if (remove_item(&item))
        {
            fprintf(stderr, "Consumer report error condition\n");
        }
        else
        {
            printf("Consumer consumed %d\n", item);
        }
        /* release the mutex lock */
        mthread_mutex_unlock(&mutex);
        /* signal empty */
        mthread_sem_post(&empty);
    }
}

int main(int argc, char *argv[])
{
    /* Loop counter */
    int i;

    /* Verify the correct number of arguments were passed in */
    if (argc != 4)
    {
        fprintf(stderr, "USAGE:./main.out <Sleep Time> <Num of Producers> <Num of Consumers>\n");
        exit(EXIT_FAILURE);
    }

    int mainSleepTime = atoi(argv[1]); /* Time in seconds for main to sleep */
    int numProd       = atoi(argv[2]); /* Number of producer threads */
    int numCons       = atoi(argv[3]); /* Number of consumer threads */

    printf("-------------------------------------------\n");
    printf("Enter Testcases - Thread Semaphores\n");
    printf("-------------------------------------------\n");

    /* Initialize the app */
    initializeData();

    /* Create the producer threads */
    for (i = 0; i < numProd; i++)
    {
        /* Create the thread */
        MCHECK(mthread_create(&tid, NULL, producer, NULL));
        printf("Producer thread %d created\n", i);
    }

    /* Create the consumer threads */
    for (i = 0; i < numCons; i++)
    {
        /* Create the thread */
        MCHECK(mthread_create(&tid, NULL, consumer, NULL));
        printf("Consumer Thread %d created\n", i);
    }

    /* Sleep for the specified amount of time in milliseconds */
    sleep(mainSleepTime);

    /* Exit the program */
    printf("Exit the program\n");
    exit(0);
}