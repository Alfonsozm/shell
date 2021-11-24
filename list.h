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
} list_t;

/**
 * Creates and reserves memory for an empty list_t.
 * 
 * @return a pointer to the newly created list_t.
 */
list_t *createEmptyList();

/**
 * Returns the void* stored in the first node_t of the list_t.
 * 
 * @param list the specific list_t.
 * @return the specific void*.
 */
void *getFirstInfo(list_t const *list);

/**
 * Returns the void* stored in the last node_t of the list_t.
 * 
 * @param list the specific list_t.
 * @return the specific void*.
 */
void *getLastInfo(list_t const *list);

/**
 * Checks whether a list_t is empty.
 * 
 * @param list the specific list_t.
 * @return true if the list is empty, in any other case returns false.
 */
int isEmpty(list_t const *list);

/**
 * Adds a void* to the list_t.
 * 
 * @param list the specific list.
 * @param info the specific void*.
 */
void addInfo(list_t *list, void *info);

/**
 * Returns how many void* are stored in the list.
 * 
 * @param list the specific list_t.
 * @return the amount of void* stored in this list_t.
 */
int getCount(list_t const *list);

#endif