#ifndef LL_HEADER
#define LL_HEADER

typedef struct node {
    double x;
    double y;
    struct node *next;
} node;

typedef struct list {
    node *head;
    int ct;
} list;

list *ll_new();
void ll_add(double x, double y, list *list);
void ll_free(list *list);

#endif
