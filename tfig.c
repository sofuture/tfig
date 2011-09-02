#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    printf("tfig -x xMin -X xMax -y yMin -Y yMax");
    printf(" -w width -h height -s symbol\n");
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
    settings* s = init_settings();
    while((c = getopt(argc, argv, "x:X:y:Y:h:w:s:?")) != -1){
        switch (c) {
            case 'x':
                s->xmin = atoi(optarg); break;
            case 'X':
                s->xmax = atoi(optarg); break;
            case 'y':
                s->ymin = atoi(optarg); break;
            case 'Y':
                s->ymax = atoi(optarg); break;
            case 'w':
                s->width = atoi(optarg); break;
            case 'h':
                s->height = atoi(optarg); break;
            case 's':
                s->sym = (char *) optarg; break;
            case ':':
            default:
                usage();
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
static List *load_data(){
    List *list = emptylist();
    int x, y, n, i = 0;

    while(scanf("%d", &n) == 1) {
        if(i % 2 == 0) {
            x = n;
        } else { 
            y = n;
            add(x, y, list);
        }
        i++;
    }

    if(DEBUG){
        Node *c;
        c = list->head;
        while(c->next != NULL){
            printf("pt: %d %d\n", c->x, c->y);
            c = c->next;
        }
    }

    return list;
}

/*
 * draw out a graph of the data that we have loaded
 */
static void draw_data(List *dat, settings *set){
    int dp[set->height][set->width];

    /* is there really not a better way to do this?
     * i don't believe that for a second!
     */
    for(int i=0; i < set->height; i++)
        for(int j=0; j < set->width; j++)
            dp[i][j] = 0;
    
    /*
     * load the data points into a form we can draw from more easily
     */
    Node *c = dat->head;
    while(c->next != NULL){
        if(c->y < set->height && c->x <= set->width){
            dp[c->y][c->x] = 1;
        }
        c = c->next;
    }

    for(int i = 0; i < set->height + 1; i++) {
        for(int j = 0; j < set->width + 1; j++) {

            /* borders */
            if(j == 0) {
                if(i == set->height) printf("+");
                else printf("|");
            } else if(i == set->height) {
                printf("-");
            } else {

                /* data points */
                if(dp[set->height-(i+1)][j-1])
                    printf("%s", set->sym);
                else
                    printf(" ");
            }
        }
        printf("\n");
    }
}

int main (int argc, char **argv) {
    settings *set = parse_opts(argc, argv);

    List *dat = load_data();

    draw_data(dat, set);

    destroy(dat);
    free(set);

    return 0;
}

