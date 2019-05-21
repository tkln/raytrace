#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "fb.h"
#include "ray.h"
#include "vecmat/vec3f.h"

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

enum material_type {
    MATERIAL_LAMBERTIAN,
    MATERIAL_METAL,
};

struct material {
    enum material_type type;
    struct color albedo;
};

struct sphere {
    struct material mat;
    struct vec3f orig;
    float r;
} spheres[] = {
    {
        .mat = {
            .type = MATERIAL_LAMBERTIAN,
            .albedo = { 0.5f, 0.5f, 0.5f },
        },
        .orig = { 0.0f, -100.5f, -1.0f },
        .r = 100.0f,
    }, {
        .mat = {
            .type = MATERIAL_METAL,
            .albedo = { 0.3f, 0.5f, 0.7f },
        },
        .orig = { -1.0f, 0.0f, -1.0f },
        .r = 0.5f,
    }, {
        .mat = {
            .type = MATERIAL_METAL,
            .albedo = { 0.3f, 0.7f, 0.5f },
        },
        .orig = { 0.0f, 0.0f, -1.0f },
        .r = 0.5f,
    }, {
        .mat = {
            .type = MATERIAL_LAMBERTIAN,
            .albedo = { 0.7f, 0.3f, 0.5f },
        },
        .orig = { 1.0f, 0.0f, -1.0f },
        .r = 0.5f,
    },
};

struct hit {
    struct material mat;
    struct vec3f p;
    struct vec3f n;
};

float hit_sphere(struct vec3f center, float r, struct ray ray)
{
    struct vec3f oc = vec3f_sub(ray.orig, center);
    float a = vec3f_dot(ray.dir, ray.dir);
    float b = 2.0f * vec3f_dot(oc, ray.dir);
    float c = vec3f_dot(oc, oc) - r * r;
    float discriminant = b * b - 4 * a * c;
    float t;
    if (discriminant < 0.0f)
        return -1.0f;
    t = (-b - sqrtf(discriminant)) / (2.0f * a);
    if (t > 0.0f)
        return t;
    t = (-b + sqrtf(discriminant)) / (2.0f * a);
    if (t > 0.0f)
        return t;
}

static bool test_hit(struct ray ray, struct hit *hit)
{
    size_t min_i;
    float min_t = FLT_MAX;
    size_t i;
    float t;

    for (i = 0; i < ARRAY_LEN(spheres); ++i) {
        t = hit_sphere(spheres[i].orig, spheres[i].r, ray);
        if (t <= 0.0f)
            continue;
        if (t < min_t) {
            min_t = t;
            min_i = i;
        }
    }

    if (min_t != FLT_MAX) {
        hit->p = ray_point_along(ray, min_t);
        hit->n = vec3f_normalized(vec3f_sub(hit->p, spheres[min_i].orig));
        hit->mat = spheres[min_i].mat;
        return true;
    }
    return false;
}

static inline struct vec3f random_in_unit_sphere(void)
{
    struct vec3f p;

    do {
        p = vec3f_init(drand48(), drand48(), drand48());
        p = vec3f_sub(vec3f_muls(p, 2.0f), vec3f_ones);
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

static inline struct color color_mul(struct color a, struct color b)
{
    struct vec3f s = vec3f_mul(*(struct vec3f *)&a, *(struct vec3f *)&b);
    return *(struct color *)&s;
}

struct vec3f vec3f_lerp(struct vec3f a, struct vec3f b, float t)
{
    return vec3f_add(vec3f_muls(a, 1.0f - t), vec3f_muls(b, t));
}

static inline struct vec3f vec3f_reflect(struct vec3f v, struct vec3f n)
{
    return vec3f_sub(v ,vec3f_muls(vec3f_muls(n, vec3f_dot(v, n)), 2.0f));
}

static struct color trace(struct ray ray, int n_bounce)
{
    struct vec3f unit_dir = vec3f_normalized(ray.dir);
    struct vec3f target;
    struct color c;
    struct vec3f cm;
    struct hit hit;
    bool is_hit = false;
    const int max_bounces = 100;
    float t;

    if (n_bounce < max_bounces)
        is_hit = test_hit(ray, &hit);

    if (is_hit) {
        if (hit.mat.type == MATERIAL_LAMBERTIAN) {
            ray.orig = hit.p;
            ray.dir = vec3f_add(hit.n, random_in_unit_sphere());
            c = trace(ray, n_bounce + 1);
            return color_mul(c, hit.mat.albedo);
        } else {
            ray.orig = hit.p;
            ray.dir = vec3f_reflect(ray.dir, hit.n);
            c = trace(ray, n_bounce + 1);
            if (vec3f_dot(ray.dir, hit.n) > 0.0f)
                return color_mul(c, hit.mat.albedo);
        }
    }

    /* Background color */
    t = 0.5f * (unit_dir.y + 1.0f);
    cm = vec3f_lerp(vec3f_ones, vec3f_init(0.5f, 0.7f, 1.0f), t);

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
    struct timespec t_start, t_end, delta;
    struct camera cam;
    int x, y, s;
    int ns = 100;
    int err;

    static DEFINE_FB(fb, 640, 480);

    struct ray ray;
    struct color c;
    float u, v;

    camera_init(&cam, fb.w, fb.h, 4.0f);

    err = clock_gettime(CLOCK_MONOTONIC, &t_start);
    if (err == -1)
        perror("clock_gettime");
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
        if (y % 10 == 0)
            fprintf(stderr, "%.2f%%\r", 100.0f * y / fb.h);
    }
    fprintf(stderr, "\n");

    err = clock_gettime(CLOCK_MONOTONIC, &t_end);
    if (err == -1)
        perror("clock_gettime");

    delta.tv_sec = t_end.tv_sec - t_start.tv_sec;
    delta.tv_nsec = t_end.tv_nsec - t_start.tv_nsec;
    fprintf(stderr, "Done in %g s\n", delta.tv_sec + delta.tv_nsec * 1e-9);

    fb_print_ppm(&fb);

    return EXIT_SUCCESS;
}
