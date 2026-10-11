#include <stdint.h>
#include <stdlib.h>

#include "ll.h"

list *ll_new(void) {
    return calloc(1, sizeof(list));
}

int ll_add(list *lp, double x, double y) {
    if (lp->ct == lp->cap) {
        size_t ncap = lp->cap ? lp->cap * 2 : 64;
        point *np;

        if (ncap < lp->cap || ncap > SIZE_MAX / sizeof(point))
            return -1;
        np = realloc(lp->pts, ncap * sizeof(point));
        if (!np)
            return -1;
        lp->pts = np;
        lp->cap = ncap;
    }

    lp->pts[lp->ct].x = x;
    lp->pts[lp->ct].y = y;
    lp->ct++;
    return 0;
}

void ll_free(list *lp) {
    if (!lp)
        return;
    free(lp->pts);
    free(lp);
}
