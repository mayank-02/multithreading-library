#include "mthread.h" 

typedef struct node {
    mthread *thd;
    struct node *next;
} node;

typedef struct queue {
    int count;
    node *head;
    node *tail;
} queue;

void initialize(queue *q);
int isempty(queue *q);
void enqueue(queue *q, mthread *thd);
mthread *dequeue(queue *q);
void display(queue *q);
mthread *search_on_tid(queue *q, unsigned long int tid);
void destroy(queue *q);