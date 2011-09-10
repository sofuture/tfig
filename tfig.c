#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "ll.h"

typedef char bool;
#define true 1
#define false 0

int win_width, win_height, win_mode;

static int _getdims(int fd) {
#ifdef TIOCGSIZE
    struct ttysize ts;
    if(ioctl(fd, TIOCGSIZE, &ts)) { perror(""); return -1; }
    if(ts.ts_cols > 0) { 
        win_width = ts.ts_cols;
        win_height = ts.ts_lines;
		win_mode = 1 + fd;
		return 0;
    }
#elif defined(TIOCGWINSZ)
    struct winsize ws;
    if(ioctl(fd, TIOCGWINSZ, &ws)) { perror(""); return -1; }
    if(ws.ws_col > 0) {
        win_width = ws.ts_col;
        win_height = ws.ts_row;
        win_mode = 1 + fd;
		return 0;
    }
#endif
    return -1;
}

static void getdims() {
    int retval, rows, cols;
    char foo[100];
    FILE *f;
    win_mode = 0;
    retval = 0;
#define GETCOLS(x) \
    if(isatty(x) && ((retval = _getdims(x)) != -1)) return; 
    GETCOLS(STDIN_FILENO)
    GETCOLS(STDOUT_FILENO)
    GETCOLS(STDERR_FILENO)
    if(getenv("COLUMNS") && getenv("LINES")) {
		win_width = atoi(getenv("COLUMNS"));
		win_height= atoi(getenv("LINES"));
		if(win_width > 0 && win_height > 0) { win_mode = 4; return; }
	}
#define STTY(x, y) \
    if((f = popen("stty " x " size", "r"))!=NULL) { \
        if(fscanf(f, "%d %d", &rows, &cols) > 0 && cols>0) \
            { win_height = rows; win_width = cols; win_mode = y; return; }\
    }
    STTY("-f /dev/stderr", 5)
    STTY("", 6)
    STTY("-f /dev/stdout", 7)
    win_width = win_height = 0;
    win_mode = 8;
    return;
}

typedef struct settings {
    int xmin;
    int xmax;
    int ymin;
    int ymax;
    int height;
    int width;
    bool asize;
    char* sym;
} settings;

/* print out usage */
static void usage(const char * const prog){
    fprintf(stderr, "%s -x xMin -X xMax -y yMin -Y yMax"
                    " -w width -h height -s symbol\n", prog);
    exit(1);
}

/* alloc and set defaults for settings struct */
static settings *init_settings(){
    settings *st = (settings *) malloc(sizeof(settings));
    getdims();
#ifdef DEBUG 
    printf("window settings: width= %d height= %d mode= %d\n", 
        win_width, win_height, win_mode);
#endif
    st->height = (win_height == 0 ? 20 : win_height - 1);
    st->width = (win_width == 0 ? 60 : win_width - 1);
    st->xmin = 0;
    st->xmax = st->height;
    st->ymin = 0;
    st->ymax = st->width;
    st->asize = true;
    st->sym = "#";
    return st;
}

/* parse getopts for graph settings */
static settings *parse_opts(int argc, char **argv){
    int c = 0;
    settings *s = init_settings();

#if defined(__linux__)
    extern char* optarg;
#endif

    bool specdim = false;

    while((c = getopt(argc, argv, "x:X:y:Y:h:w:s:?")) != -1){
        switch (c) {
            case 'x': s->xmin = atoi(optarg); specdim = true; break;
            case 'X': s->xmax = atoi(optarg); specdim = true; break;
            case 'y': s->ymin = atoi(optarg); specdim = true; break;
            case 'Y': s->ymax = atoi(optarg); specdim = true; break;
            case 'w': s->width = atoi(optarg); break;
            case 'h': s->height = atoi(optarg); break;
            case 's': s->sym = (char *) optarg; break;
            case ':':
            default: usage(argv[0]);
        }
    }

    // don't autoscale if dimensions specified
    s->asize = !specdim;
    if(s->asize) {
        s->xmin = 0; s->xmax = 0;
        s->ymin = 0; s->ymax = 0;
    }

#ifdef DEBUG
    printf("x min: %d\n", s->xmin);
    printf("x max: %d\n", s->xmax);
    printf("y min: %d\n", s->ymin);
    printf("y max: %d\n", s->ymax);
    printf("height: %d\n", s->height);
    printf("width: %d\n", s->width);
    printf("symbol: %s\n", s->sym);
#endif

    return s;
}

/*
 * load rows of data from stdin
 * assume we have <size> rows
 * assume we are given an even number of integers
 */
static list *load_data(settings *s){
    list *list = ll_new();
    double x = 0.0, y = 0.0, n = 0.0;
    int i = 0;

    while(scanf("%lf", &n) == 1) {
        if(i % 2 == 0) {
            x = n;
        } else { 
            y = n;
            ll_add(x, y, list);
        }

        if(s->asize) {
            if(x > s->xmax) s->xmax = (int) x + 1;
            else if(x < s->xmin) s->xmin = (int) x - 1;

            if(y > s->ymax) s->ymax = (int) y + 1;
            else if(y < s->ymin) s->ymin = (int) y - 1;
        }

        i++;
    }

#ifdef DEBUG
    for(node *c = list->head; c; c = c->next){
        printf("pt: %f %f\n", c->x, c->y);
    }

    if(s->asize){
        printf("auto size results:\n");
        printf("x min: %d\n", s->xmin);
        printf("x max: %d\n", s->xmax);
        printf("y min: %d\n", s->ymin);
        printf("y max: %d\n", s->ymax);
    }
#endif
    return list;
}

/* draw out a graph of the data that we have loaded */
static void draw_data(list *dat, settings *set){
    bool dp[set->height][set->width];
    memset(dp, 0, set->width * set->height * sizeof(bool));

    /* calculate 'pixel' height and width */
    double dp_width = (set->xmax - set->xmin)/(double) set->width;
    double dp_height = (set->ymax - set->ymin)/(double) set->height;

    // load the data points into a form we can draw from more easily
    for(node *c = dat->head; c; c = c->next){

        // only care about points we can see
        if(c->x >= set->xmin && c->x < set->xmax &&
                c->y >= set->ymin && c->y < set->ymax) {

            // find out which cell to place a mark in
            int px = (int) floor((c->x - set->xmin) / dp_width);
            int py = (int) floor((c->y - set->ymin) / dp_height);
            dp[py][px] = true;

#ifdef DEBUG
            printf("pts: (x:%f y:%f) -> (px:%d py:%d)\n",
                c->x, c->y, px, py);
#endif
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

    list *dat = load_data(set);

    draw_data(dat, set);

    ll_free(dat);
    free(set);

    return 0;
}

