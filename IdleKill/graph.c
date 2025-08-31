#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include "graph.h"

Graph* graph_create(int capacity) {
    Graph *graph = malloc(sizeof(Graph));
    if (!graph) return NULL;
    
    graph->nodes = calloc(capacity, sizeof(char *));
    graph->edges = calloc(capacity, sizeof(int *));
    
    if (!graph->nodes || !graph->edges) {
        free(graph->nodes);
        free(graph->edges);
        free(graph);
        return NULL;
    }
    
    for (int i = 0; i < capacity; i++) {
        graph->edges[i] = calloc(capacity, sizeof(int));
        if (!graph->edges[i]) {
            // Cleanup already allocated memory
            for (int j = 0; j < i; j++) {
                free(graph->edges[j]);
            }
            free(graph->nodes);
            free(graph->edges);
            free(graph);
            return NULL;
        }
    }
    
    graph->num_nodes = 0;
    graph->capacity = capacity;
    graph->size = 0;
    return graph;
}

bool graph_has_node(Graph *graph, const char *node) {
    if (!graph || !node) return false;
    
    for (int i = 0; i < graph->num_nodes; i++) {
        if (graph->nodes[i] && strcmp(graph->nodes[i], node) == 0) {
            return true;
        }
    }
    return false;
}

void graph_add_node(Graph* graph, const char* node) {
    if (!graph || !node || graph->num_nodes >= graph->capacity) return;
    if (graph_has_node(graph, node)) return;

    graph->nodes[graph->num_nodes] = strdup(node);
    if (!graph->nodes[graph->num_nodes]) return;
    
    graph->num_nodes++;
    graph->size++;
}

void graph_add_edge(Graph* graph, const char* from, const char* to) {
    if (!graph || !from || !to) return;
    
    int from_index = -1, to_index = -1;

    for (int i = 0; i < graph->num_nodes; i++) {
        if (graph->nodes[i] && strcmp(graph->nodes[i], from) == 0) from_index = i;
        if (graph->nodes[i] && strcmp(graph->nodes[i], to) == 0) to_index = i;
    }

    if (from_index != -1 && to_index != -1 && from_index < graph->capacity && to_index < graph->capacity) {
        graph->edges[from_index][to_index] = 1;
    }
}

int graph_size(Graph* graph) {
    return graph ? graph->size : 0;
}

void graph_remove_least_important(Graph* graph) {
    if (!graph || graph->size == 0) return;

    int index_to_remove = 0;
    int min_degree = INT_MAX;

    // Find node with minimum degree
    for (int i = 0; i < graph->num_nodes; i++) {
        if (!graph->nodes[i]) continue;
        
        int degree = 0;
        for (int j = 0; j < graph->num_nodes; j++) {
            if (!graph->nodes[j]) continue;
            degree += graph->edges[i][j];
            degree += graph->edges[j][i];
        }
        
        if (degree < min_degree) {
            min_degree = degree;
            index_to_remove = i;
        }
    }

    // Remove the node
    free(graph->nodes[index_to_remove]);
    graph->nodes[index_to_remove] = NULL;

    // Shift remaining nodes
    for (int i = index_to_remove; i < graph->num_nodes - 1; i++) {
        graph->nodes[i] = graph->nodes[i + 1];
        for (int j = 0; j < graph->capacity; j++) {
            graph->edges[i][j] = graph->edges[i + 1][j];
            graph->edges[j][i] = graph->edges[j][i + 1];
        }
    }

    // Clear the last node position
    graph->nodes[graph->num_nodes - 1] = NULL;
    for (int j = 0; j < graph->capacity; j++) {
        graph->edges[graph->num_nodes - 1][j] = 0;
        graph->edges[j][graph->num_nodes - 1] = 0;
    }

    graph->num_nodes--;
    graph->size--;
}

void graph_free(Graph* graph) {
    if (!graph) return;
    
    for (int i = 0; i < graph->capacity; i++) {
        free(graph->nodes[i]);
        free(graph->edges[i]);
    }
    free(graph->nodes);
    free(graph->edges);
    free(graph);
}
