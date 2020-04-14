/**
 * @file interrupt.c
 * @brief To handle timers
 * @author Mayank Jain
 * @bug No known bugs
 */

#include <signal.h>
#include <stdlib.h>
#include "interrupt.h"

/**
 * @brief Enable interrupts
 * @param[in] timer Pointer to timer
 * @note The timer is set for TIMER nanoseconds defined in the header
 * @note At each expiration of the timer, a SIGALRM is sent
 */
void interrupt_enable(mthread_timer_t *timer) {
    /* Enable timer (counts down in real time) */
    timer->it_value.tv_sec = 0;
    timer->it_value.tv_usec = TIMER;
	timer->it_interval.tv_sec = 0;
	timer->it_interval.tv_usec = TIMER;
    setitimer(ITIMER_VIRTUAL, timer, 0);
}

/**
 * @brief Disable interrupts
 * @param[in] timer Pointer to timer
 */
void interrupt_disable(mthread_timer_t *timer) {
    /* Disable timer */
    timer->it_value.tv_sec = 0;
    timer->it_value.tv_usec = 0;
	timer->it_interval.tv_sec = 0;
   	timer->it_interval.tv_usec = 0;
    setitimer(ITIMER_VIRTUAL, timer, 0);
}