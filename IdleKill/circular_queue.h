#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

typedef struct CircularQueue {
    char **pids;
    int front;
    int rear;
    int size;
    int capacity;
} CircularQueue;

CircularQueue *cq_create(int capacity);
void cq_enqueue(CircularQueue *cq, const char *pid);
const char *cq_dequeue(CircularQueue *cq);
int cq_is_empty(CircularQueue *cq);

#endif
