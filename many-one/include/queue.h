#include "mthread.h"

/// Node of a queue
typedef struct node {
    /// TCB
    mthread *thd;
    /// Pointer to next node
    struct node *next;
} node;

/// Queue
typedef struct queue {
    /// Count of nodes in the queue
    int count;
    /// Pointer to head node
    node *head;
    /// Pointer to tail node
    node *tail;
} queue;

void initialize(queue *q);

int isempty(queue *q);

void enqueue(queue *q, mthread *thd);

mthread *dequeue(queue *q);

int getcount(queue *q);

void display(queue *q);

mthread *search_on_tid(queue *q, pid_t tid);

void destroy(queue *q);