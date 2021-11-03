#ifndef list_h
#define list_h

#include <stdlib.h>

typedef struct node_t {
    void *info;
    struct node_t *previous;
    struct node_t *next;
} node_t;

typedef struct list_t {
    node_t *first;
    node_t *last;
    int count;
    int totalAdded;
} list_t;

list_t *createEmptyList();

void *getFirstInfo(list_t const *list);

void *getLastInfo(list_t const *list);

int isEmpty(list_t const *list);

int addInfo(list_t *list, void *info);

int getCount(list_t const *list);

int getTotalAdded(list_t const *list);

void *getByIndex(list_t const *list, int i);

#endif