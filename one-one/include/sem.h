#ifndef _SEM_H_
#define _SEM_H_
#include <stdatomic.h>
#include <stdint.h>
#include "mutex.h"

typedef struct {
    uint32_t value;
} mthread_sem_t;


int thread_sem_init(mthread_sem_t *sem, uint32_t initval);
int thread_sem_wait(mthread_sem_t *sem);
int thread_sem_post(mthread_sem_t *sem);

#endif