
#ifndef HP_KEYS_H
#define HP_KEYS_H

#include "lu/log.h"
#include "lu/gl.h"
#include "lu/memory.h"

#include "glad.h"
#include <GLFW/glfw3.h>


typedef struct keys {
    char *name;
    int keys[2];
    int mods[2];
    float force;
    float kv;
    float kx;
    float limits[2];
    size_t index;
} keys;

int set_keys(lulog *log, keys *keys, const char *name,
        int key1, int mod1, int key2, int mod2,
        float force, float kv, float kx, float lo, float hi,
        size_t index);

typedef struct variables {
    double pressed[2];
    double released[2];
    float v;
} variables;

typedef struct control {
    keys k;
    variables v;
} control;

typedef struct luary_control {
    control *c;
    lumem mem;
} luary_control;

typedef struct user_action {
    lulog *log;
    GLFWwindow *window;
    luary_control *controls;
} user_action;

int luary_control_mk(lulog *log, luary_control **controls, size_t n);
int luary_control_free(luary_control **controls, int prev_status);
int luary_control_res(lulog *log, luary_control *controls, size_t n);
int luary_control_push(lulog *log, luary_control *controls, keys *keys, float x);

int keys_free(user_action *action);

int control_update(lulog *log, double dt, control *control, float *x);
int controls_update(lulog *log, double dt, luary_control *controls, float *variables);

#endif
