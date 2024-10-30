#ifndef PROGRAMGRAPH_H
#define PROGRAMGRAPH_H

#include "execute.h"

struct PROGRAM_GRAPH_NODE {
    struct STMT* statement;
    struct PROGRAM_GRAPH_NODE* next;
};

// Function to create a new program graph node
struct PROGRAM_GRAPH_NODE* create_program_graph_node(struct STMT* stmt) {
    struct PROGRAM_GRAPH_NODE* new_node = malloc(sizeof(struct PROGRAM_GRAPH_NODE));
    new_node->statement = stmt;
    new_node->next = NULL;
    return new_node;
}

// Function to add a node to the program graph
void add_program_graph_node(struct PROGRAM_GRAPH_NODE** head, struct STMT* stmt) {
    struct PROGRAM_GRAPH_NODE* new_node = create_program_graph_node(stmt);
    new_node->next = *head;
    *head = new_node;
}

#endif // PROGRAMGRAPH_H
