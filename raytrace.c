#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "fb.h"
#include "ray.h"
#include "vecmat/vec3f.h"

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

struct sphere {
    struct vec3f orig;
    float r;
} spheres[] = {
    {
        .orig = { -0.5f, 0.0f, -0.5f },
        .r = 0.5f,
    }, {
        .orig = { 0.0f, 0.0f, -1.0f },
        .r = 0.5f,
    }, {
        .orig = { 0.5f, 0.0f, -1.5f },
        .r = 0.5f,
    },
};

struct hit {
    struct vec3f p;
    struct vec3f n;
};

float hit_sphere(struct vec3f center, float r, struct ray ray)
{
    struct vec3f oc = vec3f_sub(ray.orig, center);
    float a = vec3f_dot(ray.dir, ray.dir);
    float b = 2.0f * vec3f_dot(oc, ray.dir);
    float c = vec3f_dot(oc, oc) - r * r;
    float discriminant = b*b - 4 * a * c;
    if (discriminant < 0.0f)
        return -1.0f;
    return (-b - sqrtf(discriminant)) / (2.0f * a);
}

static bool test_hit(struct ray ray, struct hit *hit)
{
    size_t min_i;
    float min_t = FLT_MAX;
    size_t i;
    float t;

    for (i = 0; i < ARRAY_LEN(spheres); ++i) {
        t = hit_sphere(spheres[i].orig, spheres[i].r, ray);
        if (t < 0.0f)
            continue;
        if (t < min_t) {
            min_t = t;
            min_i = i;
        }
    }

    if (min_t != FLT_MAX) {
        hit->p = ray_point_along(ray, min_t);
        hit->n = vec3f_normalized(vec3f_sub(hit->p, spheres[min_i].orig));
        return true;
    }
    return false;
}

static struct color get_ray_color(struct ray ray)
{
    struct vec3f unit_dir = vec3f_normalized(ray.dir);
    struct vec3f c;
    struct hit hit;
    bool is_hit;
    float t;

    is_hit = test_hit(ray, &hit);
    if (is_hit) {
        c = vec3f_muls(vec3f_adds(hit.n, 1.0f), 0.5f);
        return (struct color) { c.x, c.y, c.z };
    }

    /* Background color */
    t = 0.5f * (unit_dir.y + 1.0f);
    c = vec3f_add(vec3f_init(1.0f - t, 1.0f - t, 1.0f - t),
                  vec3f_muls(vec3f_init(0.5f, 0.7f, 1.0f), t));

    return (struct color){ c.x, c.y, c.z, 1.0f };
}


int main(void)
{
    int x, y;

    static DEFINE_FB(fb, 480, 240);

    struct vec3f lower_left_corner = { -2.0f, -1.0f, -1.0f };
    struct vec3f horizontal = { 4.0f, 0.0f, 0.0f };
    struct vec3f vertical = { 0.0f, 2.0f, 0.0f };
    struct vec3f origin = { 0.0f, 0.0f, 0.0f };

    struct ray ray;
    float u, v;

    for (y = 0; y < fb.h; ++y) {
        for (x = 0; x < fb.w; ++x) {
            u = (float) x / fb.w;
            v = (float) y / fb.h;
            ray.orig = origin;
            ray.dir = vec3f_add3(vec3f_muls(horizontal, u),
                                 vec3f_muls(vertical, v),
                                 lower_left_corner);

            fb_putcolor(&fb, x, y, get_ray_color(ray));
        }
    }

    fb_print_ppm(&fb);
}
