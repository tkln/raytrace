#include <float.h>
#include <math.h>
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
        .orig = { 0.0f, -7.5f, -1.5f },
        .r = 7.0f,
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

static inline struct vec3f random_in_unit_sphere(void)
{
    struct vec3f p;

    do {
        p = vec3f_muls(vec3f_sub(vec3f_init(drand48(), drand48(), drand48()),
                                 vec3f_ones), 2.0f);
    } while (vec3f_norm2(p) >= 1.0f);

    return p;
}

/* TODO These break strict aliasing */
static inline struct color color_add(struct color a, struct color b)
{
    struct vec3f s = vec3f_add(*(struct vec3f *)&a, *(struct vec3f *)&b);
    return *(struct color *)&s;
}

static inline struct color color_muls(struct color c, float x)
{
    struct vec3f s = vec3f_muls(*(struct vec3f *)&c, x);
    return *(struct color *)&s;
}

static struct color trace(struct ray ray, int n_bounce)
{
    struct vec3f unit_dir = vec3f_normalized(ray.dir);
    struct vec3f target;
    struct color c;
    struct vec3f cm;
    struct hit hit;
    bool is_hit;
    float t;
    const int max_bounce = 10;

    is_hit = test_hit(ray, &hit);
    if (is_hit && n_bounce < max_bounce) {
        target = vec3f_add3(hit.p, hit.n, random_in_unit_sphere());
        ray.orig = hit.p;
        ray.dir = vec3f_sub(target, hit.p);
        c = trace(ray, n_bounce + 1);
        return color_muls(c, 0.5f);
    }

    /* Background color */
    t = 0.5f * (unit_dir.y + 1.0f);
    cm = vec3f_add(vec3f_init(1.0f - t, 1.0f - t, 1.0f - t),
                   vec3f_muls(vec3f_init(0.5f, 0.7f, 1.0f), t));

    return (struct color){ cm.x, cm.y, cm.z, 1.0f };
}

struct camera {
    struct vec3f bottom_left;
    struct vec3f horiz;
    struct vec3f vert;
    struct vec3f orig;
};

void camera_init(struct camera *cam, int fb_w, int fb_h, float viewport_w)
{
    float ar = fb_w / (float)fb_h;
    *cam = (struct camera) {
        .bottom_left    = { -viewport_w / 2.0f, -1.0f, -1.0f },
        .horiz          = { viewport_w, 0.0f, 0.0f },
        .vert           = { 0.0f, viewport_w / ar, 0.0f },
        .orig           = { 0.0f, 0.0f, 0.0f }
    };
}

struct ray camera_gen_ray(struct camera *cam, float u, float v)
{
    struct ray ray = {
        .orig = cam->orig,
        .dir = vec3f_add3(vec3f_muls(cam->horiz, u),
                          vec3f_muls(cam->vert, v),
                          cam->bottom_left)
    };

    return ray;
}

int main(void)
{
    struct camera cam;
    int x, y, s;
    int ns = 100;

    static DEFINE_FB(fb, 480, 240);

    struct ray ray;
    struct color c;
    float u, v;

    camera_init(&cam, fb.w, fb.h, 4.0f);

    for (y = 0; y < fb.h; ++y) {
        for (x = 0; x < fb.w; ++x) {
            c = (struct color) { 0.0f, 0.0f, 0.0f, 0.0f };
            for (s = 0; s < ns; ++s) {
                u = (x + drand48()) / fb.w;
                v = (y + drand48()) / fb.h;
                ray = camera_gen_ray(&cam, u, v);
                c = color_add(c, trace(ray, 0));
            }
            c = (struct color) { c.r / ns, c.g / ns, c.b / ns };
            c = (struct color) { sqrtf(c.r), sqrtf(c.g), sqrtf(c.b) };
            fb_putcolor(&fb, x, y, c);
        }
    }

    fb_print_ppm(&fb);
}
