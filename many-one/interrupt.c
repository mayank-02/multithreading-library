#include <signal.h>
#include <stdlib.h>
#include "interrupt.h"

void interrupt_enable(mthread_timer_t *timer) {
    /* Enable timer (counts down against the user-mode CPU time consumed by
       the process */
    timer->it_value.tv_sec = 0;
    timer->it_value.tv_usec = TIMER;
	timer->it_interval.tv_sec = 0;
	timer->it_interval.tv_usec = TIMER;
    setitimer(ITIMER_VIRTUAL, timer, 0);
}

void interrupt_disable(mthread_timer_t *timer) {
    /* Disable timer */
    timer->it_value.tv_sec = 0;
    timer->it_value.tv_usec = 0;
	timer->it_interval.tv_sec = 0;
   	timer->it_interval.tv_usec = 0;
    setitimer(ITIMER_VIRTUAL, timer, 0);
}