#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h" 

void initialize(queue *q) {
    q->count = 0;
    q->head = NULL;
    q->tail = NULL;
}

int isempty(queue *q) {
    if(q->head == NULL) {
        assert(q->count == 0);
        assert(q->tail == 0);
    }
    return (q->head == NULL);
}

void enqueue(queue *q, mthread *thd) {
    node *tmp = malloc(sizeof(node));
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

int getcount(queue *q) {
    return q->count;
}

void display(queue *q) {
    if(isempty(q))
        return;

    node *runner = q->head;
    printf("Queue (%d): ", q->count);
    while(runner) {
        printf("%u  ", runner->thd->tid);
        runner = runner->next;
    }
    printf("\n");
}

mthread *search_on_tid(queue *q, unsigned long int tid) {
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

void destroy(queue *q) {
    if(isempty(q))
        return;

    while(q->count--) {
        dequeue(q);
    }
    
    return;
}