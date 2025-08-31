#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <time.h>

typedef struct AppMetrics {
    char *pid;
    char *name;
    float cpu;
    float mem;
    time_t idle_time;
} AppMetrics;

typedef struct PriorityQueue {
    AppMetrics *apps;
    int size;
    int capacity;
} PriorityQueue;

PriorityQueue *pq_create(int capacity);
void pq_push(PriorityQueue *pq, AppMetrics app);
AppMetrics pq_pop(PriorityQueue *pq);
int pq_is_empty(PriorityQueue *pq);
void pq_free(PriorityQueue *pq);

#endif

