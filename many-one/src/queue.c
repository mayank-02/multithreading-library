/** 
 * @file queue.c
 * @brief Singly linked queue for thread control blocks
 * @author Mayank Jain
 * @bug No known bugs
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"

/**
 * @brief Initialise the queue
 * @param[in] q Pointer to queue
 */
void initialize(queue *q) {
    q->count = 0;
    q->head = NULL;
    q->tail = NULL;
}

/**
 * @brief Check if queue is empty
 * @param[in] q Pointer to queue
 * @return If empty, returns 1; else 0
 */
int isempty(queue *q) {
    if(q->head == NULL) {
        assert(q->count == 0);
        assert(q->tail == 0);
    }
    return (q->head == NULL);
}

/**
 * @brief Enqueue a TCB in the queue
 * @param[in] q Pointer to queue
 * @param[in] thd Pointer to TCB
 */
void enqueue(queue *q, mthread *thd) {
    node *tmp = calloc(1, sizeof(node));
    tmp->thd = thd;
    tmp->next = NULL;

    if(isempty(q)) {
        q->head = q->tail = tmp;
    }
    else {
        q->tail->next = tmp;
        q->tail = tmp;
    }
    q->count++;

    return;
}

/**
 * @brief Dequeue a TCB in the queue
 * @param[in] q Pointer to queue
 * @return Pointer to TCB dequeued; Return NULL if empty
 */
mthread *dequeue(queue *q) {
    if(isempty(q))
        return NULL;

    node *tmp = q->head;
    mthread *n = q->head->thd;

    q->head = q->head->next;
    q->count--;

    if(q->head == NULL) {
        q->tail = NULL;
    }

    free(tmp);
    return(n);
}

/**
 * @brief Get count of TCBs in queue
 * @param[in] q Pointer to queue
 * @return Count
 */
int getcount(queue *q) {
    return q->count;
}

/**
 * @brief Display TCBs in queue
 * @param[in] q Pointer to queue
 */
void display(queue *q) {
    if(isempty(q))
        return;

    node *runner = q->head;
    printf("Queue (%d): ", q->count);
    while(runner) {
        printf("%d  ", runner->thd->tid);
        runner = runner->next;
    }
    printf("\n");
}

/**
 * @brief Search for a TCB based on TID in the queue
 * @param[in] q Pointer to queue
 * @param[in] tid TID of the target thread
 * @return Pointer to target thread; Return NULL if empty
 */
mthread *search_on_tid(queue *q, pid_t tid) {
    if(isempty(q))
        return NULL;

    node *runner = q->head;
    while(runner) {
        if (runner->thd->tid == tid) {
            return runner->thd;
        }
        runner = runner->next;
    }
    return NULL;
}

/**
 * @brief Destroy the queue
 * @param[in] q Pointer to queue
 */
void destroy(queue *q) {
    if(isempty(q))
        return;

    node *curr = q->head, *next;
    for(int i = 0; i < q->count; i++) {
        next = curr->next;
        free(curr->thd);
        free(curr);
        curr = next;
    }
    
    return;
}