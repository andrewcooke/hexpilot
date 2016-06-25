
#include <string.h>

#include "lu/dynamic_memory.h"
#include "lu/status.h"
#include "lu/array_macros.h"

#include "error_codes.h"
#include "keys.h"
#include "geometry.h"


int set_keys(lulog *log, keys *keys, const char *name,
        int key0, int mod0, int key1, int mod1,
        float force, float kv, float kx, float lo, float hi,
        size_t index) {
    LU_STATUS
    LU_ALLOC(log, keys->name, strlen(name) + 1)
    strcpy(keys->name, name);
    keys->keys[0] = key0; keys->mods[0] = mod0;
    keys->keys[1] = key1; keys->mods[1] = mod1;
    keys->force = force; keys->kv = kv; keys->kx = kx;
    keys->limits[0] = lo; keys->limits[1] = hi;
    keys->index = index;
    LU_NO_CLEANUP
}

LUARY_MKBASE(control, luary_control, control, c)

int luary_pushcontrol(lulog *log, luary_control *controls, keys *keys, float x) {
    LU_STATUS
    LU_CHECK(luary_reservecontrol(log, controls, 1))
    variables v = {};
    controls->c[controls->mem.used++] = (control){*keys, v};
    LU_NO_CLEANUP
}

int init_keys(lulog *log, user_action *action) {
    LU_STATUS
    keys k = {};
    LU_CHECK(set_keys(log, &k, "+/-", 61, 1, 45, 0,
            15, 10, 0,
            0.1, 10, camera_zoom))
    LU_CHECK(luary_pushcontrol(log, action->controls, &k, 1))
    LU_CHECK(set_keys(log, &k, "left/right", 262, 0, 263, 0,
            0.3, 5, 5,
            -0.5, 0.5, ship_rotation))
    LU_CHECK(luary_pushcontrol(log, action->controls, &k, 0))
    LU_CHECK(set_keys(log, &k, "up/down",
            265, 0, 264, 0,
            2, 10, 0,
            0, 4, ship_speed))
    LU_CHECK(luary_pushcontrol(log, action->controls, &k, 0))
    LU_NO_CLEANUP
}

int free_keys(user_action *action) {
    LU_STATUS
    for (int i = 0; i < action->controls->mem.used; ++i) {
        free(action->controls->c[i].k.name);
    }
    status = luary_freecontrol(&action->controls, status);
    LU_NO_CLEANUP
}


int update_control(lulog *log, double dt, control *c, float *x) {
    LU_STATUS
    float force = 0;
    for (size_t i = 0; i < 2; ++i) {
        if (c->v.pressed[i] > c->v.released[i]) {
            force = c->k.force * (1 - 2*((int)i));
        } else {
            c->v.pressed[i] = 0;
            c->v.released[i] = 0;
        }
    }
    force -= c->v.v * c->k.kv;
    force -= *x * c->k.kx;
    c->v.v += force * dt;
    *x += c->v.v * dt;
    *x = max(c->k.limits[0], min(c->k.limits[1], *x));
    LU_NO_CLEANUP
}

int update_controls(lulog *log, double dt, luary_control *controls, float *variables) {
    LU_STATUS;
    for (size_t i = 0; i < controls->mem.used; ++i) {
        LU_CHECK(update_control(log, dt, &controls->c[i],
                &variables[controls->c[i].k.index]))
    }
    LU_NO_CLEANUP
}

int respond_to_user(lulog *log, double dt, user_action *action, float *variables) {
    LU_STATUS
    LU_CHECK(update_controls(log, dt, action->controls, variables))
    int width, height;
    glfwGetFramebufferSize(action->window, &width, &height);
    GL_CHECK(glViewport(0, 0, width, height))
    variables[buffer_x] = width; variables[buffer_y] = height;
    LU_NO_CLEANUP
}

