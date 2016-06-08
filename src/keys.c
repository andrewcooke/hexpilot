
#include <string.h>

#include "lu/dynamic_memory.h"
#include "lu/status.h"
#include "lu/array_macros.h"

#include "keys.h"


int set_keys(lulog *log, keys *keys, const char *name,
        int key0, int mod0, int key1, int mod1,
        double a, double k, double lo, double hi,
        variable_index index) {
    LU_STATUS
    free(keys->name);
    LU_ALLOC(log, keys->name, strlen(name) + 1)
    strcpy(keys->name, name);
    keys->keys[0] = key0; keys->mods[0] = mod0;
    keys->keys[1] = key1; keys->mods[1] = mod1;
    keys->a = a; keys->k = k;
    keys->limits[0] = lo; keys->limits[1] = hi;
    keys->index = index;
    LU_NO_CLEANUP
}


LUARY_MKBASE(control, luary_control, control, c)

int luary_pushcontrol(lulog *log, luary_control *controls, keys *keys, double x) {
    LU_STATUS
    LU_CHECK(luary_reservecontrol(log, controls, 1))
    variables v = {};
    controls->c[controls->mem.used++] = (control){*keys, v};
    LU_NO_CLEANUP
}


int update_control(lulog *log, double now, control *c, float *x) {
    LU_STATUS
    double dv = 0;
    for (size_t i = 0; i < 2; ++i) {
        if (c->v.pressed[i] > c->v.released[i]) {
            dv += c->k.a * (2*((int)i)-1) * (now - max(c->v.pressed[i], c->v.previous));
        } else {
            c->v.pressed[i] = 0;
            c->v.released[i] = 0;
        }
    }
    dv -= c->v.v * c->k.k * (now - c->v.previous);
    c->v.v += dv;
    *x += c->v.v * (now - c->v.previous);
    *x = max(c->k.limits[0], min(c->k.limits[1], *x));
    c->v.previous = now;
    LU_NO_CLEANUP
}

int update_controls(lulog *log, double now, luary_control *controls, float *variables) {
    LU_STATUS;
    for (size_t i = 0; i < controls->mem.used; ++i) {
        LU_CHECK(update_control(log, now, &controls->c[i],
                &variables[controls->c[i].k.index]))
    }
    LU_NO_CLEANUP
}

