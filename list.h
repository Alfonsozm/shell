#ifndef list_h
#define list_h

#include <stdlib.h>

typedef struct node_t {
    void *info;
    int id;
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

void *getFirstInfo(list_t *list);

void *getLastInfo(list_t *list);

int isEmpty(list_t *list);

int addInfo(list_t *list, void *info);

int getCount(list_t *list);

int getTotalAdded(list_t *list);

#endif