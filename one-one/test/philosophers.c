/**
 * Dining Philosophers
 * This program reads an integer from the standard input, indicating an upper 
 * bound on how many trace-steps to allow before starting to shut down. It 
 * will allow this many steps, and then shut down. The shutdown will take a few 
 * more steps. While the program is running, it will output a series of lines. 
 * Each line begins with a "Philosopher" and a number identifying the active 
 * philosopher, followed by a message.
 */

#define _REENTRANT
#include <mthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#define NTHREADS 5                         /* the number of philosophers */
#define LEFT (i + NTHREADS - 1) % NTHREADS /* philo to the left */
#define RIGHT (i + 1) % NTHREADS           /* philo to the right */
#define THINKING 1                         /* assign values to states */
#define HUNGRY 2
#define EATING 3

mthread_mutex_t M;           /* mutual exclusion for the monitor */
mthread_mutex_t outlock;     /* protects against output interleaving */
mthread_cond_t CV[NTHREADS]; /* one per philosopher */
int state[NTHREADS];         /* state of each philosopher */
int nsteps, maxsteps = 0;    /* number of steps to run this test */

int eat_count[NTHREADS] = {0}; /* number of steps for each thread */

int update_state(int i) {
    if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[i] = EATING;
        mthread_cond_signal(&CV[i]);
    }
    return 0;
}

/**
 * Call this once, before calling any of the following subprograms
 */
void chopsticks_init() {
    mthread_mutex_init(&M);
    for (int i = 0; i < NTHREADS; i++) {
        mthread_cond_init(&CV[i]);
        state[i] = THINKING;
    }
}

/**
 * Call this once after "eating". The effect is to release the
 * chopsticks at positions i and (i+1)mod 5. This should only be
 * called by a thread that has previously called chopsticks_take(i),
 * and which has not called chopsticks_put since then.
 */
void chopsticks_take(int i) {
    /* Enter cs, lock mutex */
    mthread_mutex_lock(&M);
    
    /* Set philosopher's state to HUNGRY */
    state[i] = HUNGRY;
    
    /* Update_state philosopher */
    update_state(i);            
    
    /* Loop while philosopher is hungry */
    while (state[i] == HUNGRY)  
        mthread_cond_wait(&CV[i], &M);
    
    /* Exit cs, unlock mutex */
    mthread_mutex_unlock(&M); 
}

/**
 * Call this once after "eating". The effect is to release the
 * chopsticks at positions i and (i+1)mod 5. This should only be called
 * by a thread that has previously called chopsticks_take(i), and which
 * has not called chopsticks_put since then.
 */
void chopsticks_put(int i) {
    /* Enter cs, lock mutex */
    mthread_mutex_lock(&M); 
    
    state[i] = THINKING;
    
    /* Update_state neighbors */
    update_state(LEFT); 
    update_state(RIGHT);
    
    /* Exit cs, unlock mutex */
    mthread_mutex_unlock(&M); 
}

/**
 * The subprogram trace is for testing and debugging. It allows us to
 * see what is happening, and to shut down the philosophers when
 * the program has run long enough for us to see that it seems to
 * be working
 * Also, prints out a message, for use in execution tracing
 * i = philospher ID; s = message
 */
void trace(int i, char *s) {
    mthread_mutex_lock(&outlock);
    if (strcmp(s, "Eating") == 0) {
        eat_count[i]++;
    }

    /* fprintf(stdout, "Philosopher %d: %s\n", i, s); */

    if (nsteps++ > maxsteps) {
        /* Don't exit while we are holding any chopsticks */
        if (strcmp(s, "Thinking") == 0) {
            mthread_mutex_unlock(&outlock);
            /* fprintf(stderr, "Thread done\n"); */
            mthread_exit(NULL);
        }
    }
    mthread_mutex_unlock(&outlock);
}

void *philosopher_body(void *arg) {
    int self = *(int *)arg;
    
    for (;;) {
        trace(self, "Thinking");
        chopsticks_take(self);
        trace(self, "Eating");
        chopsticks_put(self);
    }
}

int main(int argc, char **argv) {
    int i;
    int no[NTHREADS];       /* Corresponding table position numbers*/
    mthread_t th[NTHREADS]; /* IDs of the philospher threads */

    /* Initialise thread library */
    mthread_init();
    mthread_mutex_init(&outlock);
    
    /* Initialize the object chopsticks */
    chopsticks_init();      

    if (argc != 2) {
        fprintf(stdout, "Usage: ./program <number of steps to run>\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    maxsteps = atoi(argv[1]);

    /* Start up the philosopher threads */
    for (i = 0; i < NTHREADS; i++) {
        no[i] = i;
        mthread_create(&th[i], NULL, philosopher_body, (int *)&no[i]);
    }

    /* Wait for all the threads to shut down */
    for (i = 0; i < NTHREADS; i++) {
        mthread_join(th[i], NULL);
    }

    for (i = 0; i < NTHREADS; i++) {
        fprintf(stderr, "Philospher %d ate %d times\n", i, eat_count[i]);
    }

    return 0;
}