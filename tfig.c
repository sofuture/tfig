#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "ll.h"

#define DEBUG 0

typedef struct settings {
    int xmin;
    int xmax;
    int ymin;
    int ymax;
    int height;
    int width;
    char* sym;
} settings;

/*
 * print out usage
 */
static void usage(){
    fprintf(stderr, "tfig -x xMin -X xMax -y yMin -Y yMax"
                    " -w width -h height -s symbol\n");
    exit(1);
}

/*
 * alloc and set defaults for settings struct
 */
static settings *init_settings(){
    settings *st = (settings *) malloc(sizeof(settings));
    st->xmin = 0;
    st->xmax = 20;
    st->ymin = 0;
    st->ymax = 60;
    st->height = 20;
    st->width = 60;
    st->sym = "#";
    return st;
}

/*
 * parse getopts for graph settings
 */
static settings *parse_opts(int argc, char **argv){
    int c;
    extern char *optarg;
    settings *s = init_settings();

    while((c = getopt(argc, argv, "x:X:y:Y:h:w:s:?")) != -1){
        switch (c) {
            case 'x': s->xmin = atoi(optarg); break;
            case 'X': s->xmax = atoi(optarg); break;
            case 'y': s->ymin = atoi(optarg); break;
            case 'Y': s->ymax = atoi(optarg); break;
            case 'w': s->width = atoi(optarg); break;
            case 'h': s->height = atoi(optarg); break;
            case 's': s->sym = (char *) optarg; break;

            case ':':
            default: usage();
        }
    }

    if(DEBUG) {
        printf("x min: %d\n", s->xmin);
        printf("x max: %d\n", s->xmax);
        printf("y min: %d\n", s->ymin);
        printf("y max: %d\n", s->ymax);
        printf("height: %d\n", s->height);
        printf("width: %d\n", s->width);
        printf("symbol: %s\n", s->sym);
    }

    return s;
}

/*
 * load rows of data from stdin
 * assume we have <size> rows
 * assume we are given an even number of integers
 */
static list *load_data(){
    list *list = ll_new();
    float x, y, n;
    int i = 0;

    while(scanf("%f", &n) == 1) {
        if(i % 2 == 0) {
            x = n;
        } else { 
            y = n;
            ll_add(x, y, list);
        }
        i++;
    }

    if(DEBUG){
        for(node *c = list->head; c; c = c->next){
            printf("pt: %f %f\n", c->x, c->y);
        }
    }

    return list;
}

/*
 * draw out a graph of the data that we have loaded
 */
static void draw_data(list *dat, settings *set){
    char dp[set->height][set->width];
    memset(dp, 0, set->width * set->height * sizeof(char));

    /* calculate 'pixel' height and width */
    float dp_width = (set->xmax - set->xmin)/(float) set->width;
    float dp_height = (set->ymax - set->ymin)/(float) set->height;

    // load the data points into a form we can draw from more easily
    for(node *c = dat->head; c; c = c->next){

        // only care about points we can see
        if(c->x >= set->xmin && c->x < set->xmax &&
                c->y >= set->ymin && c->y < set->ymax) {

            // find out which cell to place a mark in
            int px = (int) floor((c->x - set->xmin) / dp_width);
            int py = (int) floor((c->y - set->ymin) / dp_height);
            dp[py][px] = 1;

            if(DEBUG) {
                printf("pts: (x:%f y:%f) -> (px:%d py:%d)\n",
                    c->x, c->y, px, py);
            }
        }
    }

    // arguably a sane way to draw: 'backwards'
    int i=0, j=0;
    for(i = set->height-1; i >= 0; i--){
        printf("|");
        for(j = 0; j < set->width; j++){
            if(dp[i][j]) printf("%s", set->sym);
            else printf(" ");
        }
        printf("\n");
    }

    printf("+");
    for(j = 0; j < set->width; j++){ printf("-"); }
    printf("\n");
}

int main (int argc, char **argv) {
    settings *set = parse_opts(argc, argv);

    list *dat = load_data();

    draw_data(dat, set);

    ll_free(dat);
    free(set);

    return 0;
}

