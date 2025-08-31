#include "doubly_linked_list.h"
#include <stdlib.h>
#include <string.h>

DoublyLinkedList *dll_create(int capacity) {
    DoublyLinkedList *dll = malloc(sizeof(DoublyLinkedList));
    dll->data = malloc(sizeof(char *) * capacity);
    dll->size = 0;
    dll->capacity = capacity;
    return dll;
}

void dll_append(DoublyLinkedList *dll, const char *data) {
    if (dll->size < dll->capacity) {
        dll->data[dll->size] = strdup(data);
        dll->size++;
    }
}

void dll_free(DoublyLinkedList *dll) {
    for (int i = 0; i < dll->size; i++) {
        free(dll->data[i]);
    }
    free(dll->data);
    free(dll);
}

