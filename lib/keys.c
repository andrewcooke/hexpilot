
#include <status.h>
#include <string.h>

#include "lu/memory.h"
#include "lu/arrays.h"

#include "keys.h"
#include "worlds.h"


void keys_callback(GLFWwindow *window, int key, int scancode, int act, int mods) {
    user_action *action = glfwGetWindowUserPointer(window);
    double now = glfwGetTime();
    for (size_t i = 0; i < action->controls->mem.used; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            if (key == action->controls->c[i].k.keys[j] &&
                    mods == action->controls->c[i].k.mods[j]) {
                switch(act) {
                case GLFW_PRESS:
                    ludebug(action->log, "Pressed %s (%d/%d)",
                            action->controls->c[i].k.name, key, mods);
                    action->controls->c[i].v.pressed[j] = now;
                    break;
                case GLFW_RELEASE:
                    action->controls->c[i].v.released[j] = now;
                    break;
                }
                return;
            }
        }
    }
    if (act == GLFW_PRESS) ludebug(action->log, "Unused key (%d/%d)", key, mods);
}


int set_keys(lulog *log, keys *keys, const char *name,
		int key0, int mod0, int key1, int mod1,
		float force, float kv, float kx, float lo, float hi,
		size_t index) {
	int status = LU_OK;
	LU_ALLOC(log, keys->name, strlen(name) + 1)
	strcpy(keys->name, name);
	keys->keys[0] = key0; keys->mods[0] = mod0;
	keys->keys[1] = key1; keys->mods[1] = mod1;
	keys->force = force; keys->kv = kv; keys->kx = kx;
	keys->limits[0] = lo; keys->limits[1] = hi;
	keys->index = index;
	finally:
	return status;
}

LUARY_MKBASE(control, control, c)

int luary_control_push(lulog *log, luary_control *controls, keys *keys, float x) {
	int status = LU_OK;
	try(luary_control_res(log, controls, 1))
    				variables v = {};
	controls->c[controls->mem.used++] = (control){*keys, v};
	finally:
	return status;
}

int keys_free(user_action *action) {
	int status = LU_OK;
	for (int i = 0; i < action->controls->mem.used; ++i) {
		free(action->controls->c[i].k.name);
	}
	status = luary_control_free(&action->controls, status);
	finally:
	return status;
}


int control_update(lulog *log, double dt, control *c, float *x) {
	int status = LU_OK;
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
	finally:
	return status;
}

int controls_update(lulog *log, double dt, luary_control *controls, float *variables) {
	int status = LU_OK;;
	for (size_t i = 0; i < controls->mem.used; ++i) {
		try(control_update(log, dt, &controls->c[i], &variables[controls->c[i].k.index]));
	}
	finally:
	return status;
}


