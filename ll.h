#ifndef LL_HEADER
#define LL_HEADER

#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int x;
    int y;
    struct node *next;
} Node;

typedef struct list {
    Node *head;
    int ct;
} List;

List *emptylist();
void add(int x, int y, List *list);
void destroy(List *list);

#endif
