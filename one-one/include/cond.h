#ifndef _COND_H_
#define _COND_H_
#include <stdatomic.h>
#include "mutex.h"

typedef struct mthread_cond_t { 
    int  value;
    unsigned int previous;
} mthread_cond_t;

int thread_cond_init(mthread_cond_t *cond);
int thread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mutex);
int thread_cond_signal(mthread_cond_t *cond);

#endif