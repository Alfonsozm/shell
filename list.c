#include "list.h"

list_t *createEmptyList() {
    list_t *list = (list_t *) malloc(sizeof(list_t));
    list->first = NULL;
    list->last = NULL;
    list->count = 0;
    list->totalAdded = 0;
    return list;
}

void *getFirstInfo(list_t *list) {
    return list->first->info;
}

void *getLastInfo(list_t *list) {
    return list->last->info;
}

int isEmpty(list_t *list) {
    return list->count == 0;
}

void addInfo(list_t *list, void *info) {
    node_t *tmp = malloc(sizeof(node_t));
    tmp->info = info;
    if (isEmpty(list)) {
        list->count++;
        list->totalAdded++;
        tmp->id = list->totalAdded;
        list->first = tmp;
        list->last = tmp;
    } else {
        list->count++;
        list->totalAdded++;
        tmp->id = list->totalAdded;
        list->last->next = tmp;
        tmp->previous = list->last;
        list->last = tmp;
    }
}

int getCount(list_t *list) {
    return list->count;
}

int getTotalAdded(list_t *list) {
    return list->totalAdded;
}