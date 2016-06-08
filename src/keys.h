
#ifndef HP_KEYS_H
#define HP_KEYS_H

#include "lu/log.h"
#include "lu/vectors.h"

#include "geometry.h"


typedef struct keys {
    char *name;
    int keys[2];
    int mods[2];
    double a;
    double k;
    double limits[2];
    variable_index index;
} keys;

int set_keys(lulog *log, keys *keys, const char *name,
        int key1, int mod1, int key2, int mod2,
        double a, double k, double lo, double hi,
        variable_index index);

typedef struct variables {
    double previous;
    double pressed[2];
    double released[2];
    double v;
} variables;

typedef struct control {
    keys k;
    variables v;
} control;

typedef struct luary_control {
    control *c;
    lumem mem;
} luary_control;

int luary_mkcontroln(lulog *log, luary_control **controls, size_t n);
int luary_freecontrol(luary_control **controls, int prev_status);
int luary_reservecontrol(lulog *log, luary_control *controls, size_t n);
int luary_pushcontrol(lulog *log, luary_control *controls, keys *keys, double x);
size_t luary_sizecontrol(luary_control *buffer);

int update_control(lulog *log, double now, control *control, float *x);
int update_controls(lulog *log, double now, luary_control *controls, float *variables);

#endif
