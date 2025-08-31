#ifndef HASHMAP_H
#define HASHMAP_H

#include <time.h>

typedef struct HashMap {
    char **keys;
    time_t *values;
    time_t *last_active;
    int size;
    int capacity;
} HashMap;

HashMap *hashmap_create(int capacity);
void hashmap_put(HashMap *map, const char *key, time_t value);
time_t hashmap_get(HashMap *map, const char *key);
time_t *hashmap_get_ptr(HashMap *map, const char *key);
void hashmap_update_active_time(HashMap *map, const char *key);
time_t hashmap_get_idle_time(HashMap *map, const char *key);
void hashmap_remove(HashMap *map, const char *key);
void hashmap_free(HashMap *map);

#endif
