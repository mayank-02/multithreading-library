#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <sys/time.h>
#define TIMER   (suseconds_t) 10000

typedef struct itimerval mthread_timer_t;

void interrupt_enable(mthread_timer_t *timer);

void interrupt_disable(mthread_timer_t *timer);

#endif