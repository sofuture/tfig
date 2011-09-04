#ifndef LL_HEADER
#define LL_HEADER

#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    double x;
    double y;
    struct node *next;
} Node;

typedef struct list {
    Node *head;
    int ct;
} List;

List *emptylist();
void add(double x, double y, List *list);
void destroy(List *list);

#endif
