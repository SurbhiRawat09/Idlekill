#include "priority_queue.h"
#include <stdlib.h>
#include <string.h>

PriorityQueue *pq_create(int capacity) {
    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    pq->apps = malloc(sizeof(AppMetrics) * capacity);
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void pq_push(PriorityQueue *pq, AppMetrics app) {
    if (pq->size < pq->capacity) {
        pq->apps[pq->size++] = app;
    }
}

AppMetrics pq_pop(PriorityQueue *pq) {
    if (pq_is_empty(pq)) {
        AppMetrics empty = {NULL, NULL, 0.0, 0.0};
        return empty;
    }
    return pq->apps[--pq->size];
}

int pq_is_empty(PriorityQueue *pq) {
    return pq->size == 0;
}

void pq_free(PriorityQueue *pq) {
    free(pq->apps);
    free(pq);
}

