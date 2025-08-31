#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

typedef struct DoublyLinkedList {
    char **data;
    int size;
    int capacity;
} DoublyLinkedList;

DoublyLinkedList *dll_create(int capacity);
void dll_append(DoublyLinkedList *dll, const char *data);
void dll_free(DoublyLinkedList *dll);

#endif

