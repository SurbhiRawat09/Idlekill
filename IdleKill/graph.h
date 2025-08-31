#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

#define MAX_NODES 100

typedef struct Graph {
    char **nodes;         // Array of node names
    int **edges;          // Adjacency matrix
    int num_nodes;
    int capacity;
    int size;             // for graph_size
} Graph;

// Graph API
Graph* graph_create(int capacity);
void graph_add_node(Graph* graph, const char* pid);
bool graph_has_node(Graph *graph, const char *node);
void graph_add_edge(Graph* graph, const char* from, const char* to);
int graph_size(Graph* graph);
void graph_remove_least_important(Graph* graph);
void graph_free(Graph* graph);

#endif

