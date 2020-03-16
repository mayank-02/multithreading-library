#include "mthread.h" 

typedef struct node {
    mthread_t thd;
    struct node *next;
} node;

typedef struct queue {
    int count;
    node *head;
    node *tail;
} queue;

void initialize(queue *q);
int isempty(queue *q);
void enqueue(queue *q, mthread_t thd);
mthread_t dequeue(queue *q);
void display(queue *q);
mthread_t search_on_tid(queue *q, pid_t tid);
void destroy(queue *q);