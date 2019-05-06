#ifndef RAY_H
#define RAY_H

#include "vecmat/vec3f.h"

struct ray {
    struct vec3f orig;
    struct vec3f dir;
};

static inline struct vec3f ray_point_along(struct ray ray, float t)
{
    return vec3f_add(vec3f_muls(ray.dir, t), ray.orig);
}

#endif
