#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "fb.h"
#include "ray.h"
#include "vecmat/vec3f.h"

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

static struct color get_ray_color(struct ray ray)
{
    struct vec3f unit_dir = vec3f_normalized(ray.dir);
    struct vec3f sphere_orig = { 0, 0, -1 };
    struct vec3f c;
    struct vec3f n;
    float t;

    t = hit_sphere(sphere_orig, 0.5, ray);
    if (t > 0.0f) {
        n = vec3f_normalized(vec3f_sub(ray_point_along(ray, t),
                                       sphere_orig));
        c = vec3f_muls(vec3f_adds(n, 1.0f), 0.5f);
        return (struct color) { c.x, c.y, c.z };
    }

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
            ray.dir = vec3f_add(vec3f_add(vec3f_muls(horizontal, u),
                                          vec3f_muls(vertical, v)),
                                lower_left_corner);

            fb_putcolor(&fb, x, y, get_ray_color(ray));
        }
    }

    fb_print_ppm(&fb);
}
