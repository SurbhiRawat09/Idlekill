#include "circular_queue.h"
#include <stdlib.h>
#include <string.h>

CircularQueue *cq_create(int capacity) {
    CircularQueue *cq = malloc(sizeof(CircularQueue));
    if (!cq) return NULL;
    
    cq->pids = calloc(capacity, sizeof(char *));
    if (!cq->pids) {
        free(cq);
        return NULL;
    }
    
    cq->front = 0;
    cq->rear = -1;
    cq->size = 0;
    cq->capacity = capacity;
    return cq;
}

void cq_enqueue(CircularQueue *cq, const char *pid) {
    if (cq->size < cq->capacity) {
        cq->rear = (cq->rear + 1) % cq->capacity;
        cq->pids[cq->rear] = strdup(pid);
        cq->size++;
    }
}

const char *cq_dequeue(CircularQueue *cq) {
    if (cq_is_empty(cq)) {
        return NULL;
    }
    const char *pid = cq->pids[cq->front];
    cq->front = (cq->front + 1) % cq->capacity;
    cq->size--;
    return pid;
}

int cq_is_empty(CircularQueue *cq) {
    return cq->size == 0;
}
