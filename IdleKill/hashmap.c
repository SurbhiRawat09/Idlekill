#include "hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

HashMap *hashmap_create(int capacity) {
    HashMap *map = malloc(sizeof(HashMap));
    if (!map) return NULL;
    
    map->keys = calloc(capacity, sizeof(char *));
    map->values = calloc(capacity, sizeof(time_t));
    map->last_active = calloc(capacity, sizeof(time_t));
    
    if (!map->keys || !map->values || !map->last_active) {
        free(map->keys);
        free(map->values);
        free(map->last_active);
        free(map);
        return NULL;
    }
    
    map->size = 0;
    map->capacity = capacity;
    return map;
}

void hashmap_put(HashMap *map, const char *key, time_t value) {
    if (map->size >= map->capacity) return;
    
    for (int i = 0; i < map->size; i++) {
        if (strcmp(map->keys[i], key) == 0) {
            map->values[i] = value;
            map->last_active[i] = time(NULL);
            return;
        }
    }
    
    map->keys[map->size] = strdup(key);
    map->values[map->size] = value;
    map->last_active[map->size] = time(NULL);
    map->size++;
}

time_t hashmap_get(HashMap *map, const char *key) {
    for (int i = 0; i < map->size; i++) {
        if (strcmp(map->keys[i], key) == 0) {
            return map->values[i];
        }
    }
    return -1;
}

time_t *hashmap_get_ptr(HashMap *map, const char *key) {
    for (int i = 0; i < map->size; i++) {
        if (strcmp(map->keys[i], key) == 0) {
            return &map->last_active[i];
        }
    }
    return NULL;
}

void hashmap_update_active_time(HashMap *map, const char *key) {
    for (int i = 0; i < map->size; i++) {
        if (strcmp(map->keys[i], key) == 0) {
            map->last_active[i] = time(NULL);
            return;
        }
    }
}

time_t hashmap_get_idle_time(HashMap *map, const char *key) {
    time_t now = time(NULL);
    for (int i = 0; i < map->size; i++) {
        if (strcmp(map->keys[i], key) == 0) {
            return now - map->last_active[i];
        }
    }
    return -1;
}

void hashmap_remove(HashMap *map, const char *key) {
    for (int i = 0; i < map->size; i++) {
        if (strcmp(map->keys[i], key) == 0) {
            free(map->keys[i]);
            for (int j = i; j < map->size - 1; j++) {
                map->keys[j] = map->keys[j + 1];
                map->values[j] = map->values[j + 1];
                map->last_active[j] = map->last_active[j + 1];
            }
            map->size--;
            return;
        }
    }
}

void hashmap_free(HashMap *map) {
    for (int i = 0; i < map->size; i++) {
        free(map->keys[i]);
    }
    free(map->keys);
    free(map->values);
    free(map->last_active);
    free(map);
}
