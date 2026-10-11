#ifndef LL_HEADER
#define LL_HEADER

#include <stddef.h>

typedef struct point {
    double x;
    double y;
} point;

typedef struct list {
    point *pts;
    size_t ct;
    size_t cap;
} list;

list *ll_new(void);
int ll_add(list *lp, double x, double y);
void ll_free(list *lp);

#endif
