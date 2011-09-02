#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#define DEBUG 1

typedef struct settings {
    int xmin;
    int xmax;
    int ymin;
    int ymax;
    int height;
    int width;
    char* sym;
} settings;

typedef struct dp {
    int x;
    int y;
} dp;

static void usage(){
    printf("tfig -x xMin -X xMax -y yMin -Y yMax");
    printf(" -w width -h height -s symbol\n");
}

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

static settings* parse_opts(int argc, char **argv){
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
                s->sym = (char*) optarg; break;
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

static dp* load_data(int size){

    dp* dat = (dp *) malloc(sizeof(dp)*size);
    int n, i = 0;

    while(scanf("%d", &n) == 1) {
        if(i%2 == 0)
            dat[i/2].x = n;
        else
            dat[(i-1)/2].y = n;
        i++;
    }

    return dat;
}

static void draw_data(dp* dat, settings* set){

    printf("\n");

    for(int i = 0; i < set->height + 1; i++) {
        for(int j = 0; j < set->width + 1; j++) {
       
            /* borders */
            if(j == 0) {
                if(i == set->height) printf("+");
                else printf("|");
            } else if(i == set->height) {
                printf("-");
            } else {
                /* data */
                int y = dat[j-1].y;
                
                if(set->height - (y+1) == i)
                    printf("%s",set->sym);
                else
                    printf(" ");
            }

        }
        printf("\n");
    }
    printf("\n");
}

int main (int argc, char **argv) {
    settings* set = parse_opts(argc, argv);

    // assuming we get max - min data points (temporary)
    int size = set->ymax - set->ymin;

    dp* dat = load_data(size);

    if(DEBUG){
        for(int i=0;i<size;i++){
            printf("pt: %d %d\n", dat[i].x, dat[i].y);
        }
    }

    draw_data(dat, set);

    return 0;
}

