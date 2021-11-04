#include "list.h"

list_t *createEmptyList() {
    list_t *list = (list_t *) malloc(sizeof(list_t));
    list->first = NULL;
    list->last = NULL;
    list->count = 0;
    return list;
}

void *getFirstInfo(list_t const *list) {
    return list->first->info;
}

void *getLastInfo(list_t const *list) {
    return list->last->info;
}

int isEmpty(list_t const *list) {
    return list->count == 0;
}

void addInfo(list_t *list, void *info) {
    node_t *tmp = malloc(sizeof(node_t));
    tmp->info = info;
    tmp->previous = NULL;
    tmp->next = NULL;
    if (isEmpty(list)) {
        list->count++;
        list->first = tmp;
        list->last = tmp;
    } else {
        list->count++;
        list->last->next = tmp;
        tmp->previous = list->last;
        list->last = tmp;
    }
}

int getCount(list_t const *list) {
    return list->count;
}

void *getByIndex(list_t const *list, int index) {
    if (index >= list->count) {
        return NULL;
    } else {
        node_t *n = list->first;
        for (int i = 1; i < index; ++i) {
            n = n->next;
        }
        return n;
    }
}