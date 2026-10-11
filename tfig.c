#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <locale.h>
#include <wchar.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#include "ll.h"

#define MAX_DIM 10000

int win_width, win_height, win_mode;

static int read_winsize(int fd) {
#ifdef TIOCGWINSZ
    struct winsize ws;
    if (ioctl(fd, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        win_width = ws.ws_col;
        win_height = ws.ws_row;
        return 0;
    }
#endif
#ifdef TIOCGSIZE
    struct ttysize ts;
    if (ioctl(fd, TIOCGSIZE, &ts) == 0 && ts.ts_cols > 0 && ts.ts_lines > 0) {
        win_width = ts.ts_cols;
        win_height = ts.ts_lines;
        return 0;
    }
#endif
    return -1;
}

static int env_dim(const char *name, int *out) {
    const char *v = getenv(name);
    char *end;
    long n;

    if (!v || !v[0])
        return -1;
    errno = 0;
    n = strtol(v, &end, 10);
    if (*end != '\0' || errno == ERANGE || n <= 0 || n > INT_MAX)
        return -1;
    *out = (int)n;
    return 0;
}

static void getdims(void) {
    int cols, rows, fd;
    const int fds[3] = { STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO };
    size_t i;

    win_width = 0;
    win_height = 0;
    win_mode = 0;

    for (i = 0; i < 3; i++) {
        if (isatty(fds[i]) && read_winsize(fds[i]) == 0) {
            win_mode = 1 + fds[i];
            return;
        }
    }

    fd = open("/dev/tty", O_RDONLY | O_NOCTTY);
    if (fd >= 0) {
        if (read_winsize(fd) == 0) {
            win_mode = 4;
            close(fd);
            return;
        }
        close(fd);
    }

    if (env_dim("COLUMNS", &cols) == 0 && env_dim("LINES", &rows) == 0) {
        win_width = cols;
        win_height = rows;
        win_mode = 5;
        return;
    }

    win_mode = 8;
}

typedef struct settings {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    int height;
    int width;
    bool have_xmin;
    bool have_xmax;
    bool have_ymin;
    bool have_ymax;
    char sym[5];
    size_t sym_len;
} settings;

static void die(const char *msg) {
    fprintf(stderr, "tfig: %s\n", msg);
    exit(1);
}

/* print out usage */
static void usage(const char *prog) {
    fprintf(stderr,
        "usage: %s [-x xmin] [-X xmax] [-y ymin] [-Y ymax]\n"
        "          [-w width] [-h height] [-s symbol]\n"
        "symbol is one column wide (UTF-8 allowed); width and height are 1..%d\n"
        "bounds may be fractional and are inclusive; omitted bounds fit the data\n",
        prog, MAX_DIM);
    exit(1);
}

static int parse_int(const char *s, int *out) {
    char *end;
    long v;

    if (!s || !s[0])
        return -1;
    errno = 0;
    v = strtol(s, &end, 10);
    if (*end != '\0' || errno == ERANGE || v < INT_MIN || v > INT_MAX)
        return -1;
    *out = (int)v;
    return 0;
}

static int parse_double(const char *s, double *out) {
    char *end;
    double v;

    if (!s || !s[0])
        return -1;
    errno = 0;
    v = strtod(s, &end);
    if (*end != '\0' || errno == ERANGE || !isfinite(v))
        return -1;
    *out = v;
    return 0;
}

/* Classify non-ASCII scalars with wcwidth, which needs a UTF-8 LC_CTYPE. */
static void init_ctype(void) {
    const char *tries[] = { "C.UTF-8", "C.utf8", "en_US.UTF-8", "en_US.utf8", NULL };
    int i;

    for (i = 0; tries[i] != NULL; i++) {
        if (setlocale(LC_CTYPE, tries[i]) != NULL)
            return;
    }
    setlocale(LC_CTYPE, "");
}

/* Terminal columns for one scalar, or -1 when it cannot be represented. */
static int column_width(unsigned long cp) {
    wchar_t wc;

    if (cp > 0x10ffff)
        return -1;
    /* A 16-bit wchar_t cannot hold this scalar, so it cannot be one column here. */
    if (sizeof(wchar_t) < 4 && cp > 0xffff)
        return -1;
    wc = (wchar_t)cp;
    return wcwidth(wc);
}

static void ignore_sigpipe(void) {
    struct sigaction sa;

    memset(&sa, 0, sizeof sa);
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);
}

/*
 * Return the byte length of s if it is exactly one printable ASCII character
 * or one well-formed UTF-8 scalar that occupies one terminal column.
 * Wide, combining, and non-printing scalars return 0.
 */
static size_t single_char_len(const char *s) {
    const unsigned char *p = (const unsigned char *)s;
    size_t len, i;
    unsigned long cp;

    if (p[0] >= 0x20 && p[0] < 0x7f)
        return p[1] == '\0' ? 1 : 0;
    if (p[0] >= 0xc2 && p[0] <= 0xdf) {
        len = 2;
        cp = p[0] & 0x1f;
    } else if (p[0] >= 0xe0 && p[0] <= 0xef) {
        len = 3;
        cp = p[0] & 0x0f;
    } else if (p[0] >= 0xf0 && p[0] <= 0xf4) {
        len = 4;
        cp = p[0] & 0x07;
    } else {
        return 0;
    }
    for (i = 1; i < len; i++) {
        if ((p[i] & 0xc0) != 0x80)
            return 0;
        cp = (cp << 6) | (p[i] & 0x3f);
    }
    if (p[len] != '\0')
        return 0;
    if ((len == 3 && cp < 0x800) || (len == 4 && cp < 0x10000) ||
            (cp >= 0xd800 && cp <= 0xdfff) || cp > 0x10ffff || cp < 0xa0)
        return 0;
    if (column_width(cp) != 1)
        return 0;
    return len;
}

/* set defaults for settings, using the terminal size when it is known */
static void init_settings(settings *st) {
    getdims();
#ifdef DEBUG
    printf("window settings: width= %d height= %d mode= %d\n",
        win_width, win_height, win_mode);
#endif
    st->height = (win_height > 1 ? win_height - 1 : 20);
    st->width = (win_width > 1 ? win_width - 1 : 60);
    if (st->height > MAX_DIM)
        st->height = MAX_DIM;
    if (st->width > MAX_DIM)
        st->width = MAX_DIM;
    st->xmin = 0;
    st->xmax = 1;
    st->ymin = 0;
    st->ymax = 1;
    st->have_xmin = false;
    st->have_xmax = false;
    st->have_ymin = false;
    st->have_ymax = false;
    st->sym[0] = '#';
    st->sym[1] = '\0';
    st->sym_len = 1;
}

/*
 * Fill any bound the user did not set. A point that lands on the max
 * edge stays inside the window; draw_data clamps that bin.
 * A fully omitted axis with no samples becomes [0, 1]. When only one bound
 * is given and the data lies entirely on the wrong side of it, the window
 * extends one unit from that bound and the plot is empty.
 */
static int finalize_axis(double *mn, double *mx, bool have_min, bool have_max,
                         double lo, double hi, bool have_points,
                         const char **err) {
    double next_min = *mn;
    double next_max = *mx;

    if (!have_points) {
        if (!have_min && !have_max) {
            next_min = 0;
            next_max = 1;
        } else if (!have_max) {
            next_max = next_min + 1;
        } else if (!have_min) {
            next_min = next_max - 1;
        }
    } else {
        if (!have_min)
            next_min = (have_max && lo > next_max) ? next_max - 1 : lo;
        if (!have_max)
            next_max = (have_min && hi < next_min) ? next_min + 1 : hi;
    }

    if (next_max < next_min) {
        *err = "inverted range (max must be >= min)";
        return -1;
    }
    if (!isfinite(next_max - next_min)) {
        *err = "range is too large";
        return -1;
    }
    *mn = next_min;
    *mx = next_max;
    return 0;
}

/* parse getopts for graph settings */
static void parse_opts(int argc, char **argv, settings *s) {
    int c, v;
    double d;

    while ((c = getopt(argc, argv, "x:X:y:Y:h:w:s:?")) != -1) {
        switch (c) {
            case 'x':
                if (parse_double(optarg, &d) != 0)
                    die("invalid number for -x");
                s->xmin = d;
                s->have_xmin = true;
                break;
            case 'X':
                if (parse_double(optarg, &d) != 0)
                    die("invalid number for -X");
                s->xmax = d;
                s->have_xmax = true;
                break;
            case 'y':
                if (parse_double(optarg, &d) != 0)
                    die("invalid number for -y");
                s->ymin = d;
                s->have_ymin = true;
                break;
            case 'Y':
                if (parse_double(optarg, &d) != 0)
                    die("invalid number for -Y");
                s->ymax = d;
                s->have_ymax = true;
                break;
            case 'w':
                if (parse_int(optarg, &v) != 0)
                    die("invalid integer for -w");
                s->width = v;
                break;
            case 'h':
                if (parse_int(optarg, &v) != 0)
                    die("invalid integer for -h");
                s->height = v;
                break;
            case 's':
                s->sym_len = optarg ? single_char_len(optarg) : 0;
                if (s->sym_len == 0)
                    die("symbol must be a single one-column character");
                memcpy(s->sym, optarg, s->sym_len + 1);
                break;
            case ':':
            default:
                usage(argv[0]);
        }
    }

    if (s->width < 1 || s->height < 1 || s->width > MAX_DIM || s->height > MAX_DIM) {
        fprintf(stderr, "tfig: width and height must be between 1 and %d\n", MAX_DIM);
        exit(1);
    }

#ifdef DEBUG
    printf("x min: %g\n", s->xmin);
    printf("x max: %g\n", s->xmax);
    printf("y min: %g\n", s->ymin);
    printf("y max: %g\n", s->ymax);
    printf("height: %d\n", s->height);
    printf("width: %d\n", s->width);
    printf("symbol: %s\n", s->sym);
#endif
}

/*
 * Load pairs of numbers from stdin.
 * A trailing unpaired value is ignored and does not move the window.
 */
static list *load_data(settings *s) {
    list *data = ll_new();
    double n, x = 0.0, y;
    double min_x = 0.0, max_x = 0.0, min_y = 0.0, max_y = 0.0;
    bool have_x = false, have_points = false;
    int rc;

    if (!data)
        die("out of memory");

    while ((rc = scanf("%lf", &n)) == 1) {
        if (!isfinite(n)) {
            ll_free(data);
            die("non-finite input");
        }
        if (!have_x) {
            x = n;
            have_x = true;
            continue;
        }

        y = n;
        have_x = false;
        if (ll_add(data, x, y) != 0) {
            ll_free(data);
            die("out of memory");
        }

        if (!have_points) {
            min_x = max_x = x;
            min_y = max_y = y;
            have_points = true;
        } else {
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
    }

    if (ferror(stdin)) {
        ll_free(data);
        die("error reading input");
    }
    if (rc != EOF) {
        ll_free(data);
        die("non-numeric input");
    }
    if (have_x)
        fprintf(stderr, "tfig: trailing unpaired value ignored\n");

#ifdef DEBUG
    {
        size_t i;
        for (i = 0; i < data->ct; i++)
            printf("pt: %f %f\n", data->pts[i].x, data->pts[i].y);
    }
#endif

    {
        const char *err = NULL;
        if (finalize_axis(&s->xmin, &s->xmax, s->have_xmin, s->have_xmax,
                          min_x, max_x, have_points, &err) != 0 ||
            finalize_axis(&s->ymin, &s->ymax, s->have_ymin, s->have_ymax,
                          min_y, max_y, have_points, &err) != 0) {
            ll_free(data);
            die(err ? err : "invalid range");
        }
    }

#ifdef DEBUG
    printf("range x: %g .. %g\n", s->xmin, s->xmax);
    printf("range y: %g .. %g\n", s->ymin, s->ymax);
#endif
    return data;
}

static int bin_index(double v, double lo, double hi, int cells) {
    double span = hi - lo;
    double pos;
    int idx;

    if (cells <= 0)
        return 0;
    /* A zero-length axis has no position along it; use the middle cell. */
    if (!(span > 0.0))
        return cells / 2;
    pos = floor((v - lo) / span * (double)cells);
    if (pos >= (double)cells)
        return cells - 1;
    idx = (int)pos;
    if (idx < 0)
        idx = 0;
    if (idx >= cells)
        idx = cells - 1;
    return idx;
}

/* draw out a graph of the data that we have loaded */
static void draw_data(list *dat, settings *set) {
    size_t w = (size_t)set->width;
    size_t h = (size_t)set->height;
    size_t i;
    size_t row_cap, len;
    unsigned char *dp;
    char *row;

    if (w == 0 || h > SIZE_MAX / w || w > (SIZE_MAX - 2) / set->sym_len)
        die("dimensions too large");

    row_cap = w * set->sym_len + 2;
    dp = calloc(h * w, 1);
    row = malloc(row_cap);
    if (!dp || !row) {
        free(dp);
        free(row);
        ll_free(dat);
        die("out of memory");
    }

    for (i = 0; i < dat->ct; i++) {
        double x = dat->pts[i].x;
        double y = dat->pts[i].y;
        int px, py;

        if (x >= set->xmin && x <= set->xmax &&
                y >= set->ymin && y <= set->ymax) {
            px = bin_index(x, set->xmin, set->xmax, set->width);
            py = bin_index(y, set->ymin, set->ymax, set->height);
            dp[(size_t)py * w + (size_t)px] = 1;
#ifdef DEBUG
            printf("pts: (x:%f y:%f) -> (px:%d py:%d)\n", x, y, px, py);
#endif
        }
    }

    for (int r = set->height - 1; r >= 0; r--) {
        len = 0;
        row[len++] = '|';
        for (int c = 0; c < set->width; c++) {
            size_t idx = (size_t)r * w + (size_t)c;
            if (dp[idx]) {
                memcpy(row + len, set->sym, set->sym_len);
                len += set->sym_len;
            } else {
                row[len++] = ' ';
            }
        }
        row[len++] = '\n';
        if (fwrite(row, 1, len, stdout) != len) {
            free(row);
            free(dp);
            ll_free(dat);
            die("write failed");
        }
    }

    row[0] = '+';
    for (size_t c = 0; c < w; c++)
        row[1 + c] = '-';
    row[w + 1] = '\n';
    if (fwrite(row, 1, w + 2, stdout) != w + 2) {
        free(row);
        free(dp);
        ll_free(dat);
        die("write failed");
    }

    free(row);
    free(dp);
    if (fflush(stdout) != 0 || ferror(stdout)) {
        ll_free(dat);
        die("write failed");
    }
}

int main(int argc, char **argv) {
    settings set;
    list *dat;

    init_ctype();
    ignore_sigpipe();
    init_settings(&set);
    parse_opts(argc, argv, &set);
    dat = load_data(&set);
    draw_data(dat, &set);
    ll_free(dat);
    return 0;
}
