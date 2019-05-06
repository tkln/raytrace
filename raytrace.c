#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "fb.h"
#include "ray.h"
#include "vecmat/vec3f.h"

bool hit_sphere(struct vec3f center, float r, struct ray ray)
{
    struct vec3f oc = vec3f_sub(ray.orig, center);
    float a = vec3f_dot(ray.dir, ray.dir);
    float b = 2.0f * vec3f_dot(oc, ray.dir);
    float c = vec3f_dot(oc, oc) - r * r;
    float discriminant = b*b - 4 * a * c;
    return (discriminant > 0.0f);
}

static struct color get_ray_color(struct ray ray)
{
    struct vec3f unit_dir = vec3f_normalized(ray.dir);
    float t = 0.5f * (unit_dir.y + 1.0f);
    float ti = 1.0f - t;
    struct vec3f c;

    if (hit_sphere(vec3f_init(0, 0, -1), 0.5, ray))
        return (struct color) { 0.2f, 0.3f, 0.5f };

    c = vec3f_add(vec3f_init(ti, ti, ti),
                  vec3f_muls(vec3f_init(0.5f, 0.7f, 1.0f), t));

    return (struct color){ c.x, c.y, c.z, 1.0f };
}

int main(void)
{
    const int fb_w = 480;
    const int fb_h = 240;
    int x, y;
    struct fb fb;

    struct vec3f lower_left_corner = { -2.0f, -1.0f, -1.0f };
    struct vec3f horizontal = { 4.0f, 0.0f, 0.0f };
    struct vec3f vertical = { 0.0f, 2.0f, 0.0f };
    struct vec3f origin = { 0.0f, 0.0f, 0.0f };

    struct ray ray;
    float u, v;

    fb_init(&fb, fb_w, fb_h);
    
    for (y = 0; y < fb_h; ++y) {
        for (x = 0; x < fb_w; ++x) {
            u = (float) x / fb_w;
            v = (float) y / fb_h;
            ray.orig = origin;
            ray.dir = vec3f_add(vec3f_add(vec3f_muls(horizontal, u),
                                          vec3f_muls(vertical, v)),
                                lower_left_corner);

            fb_putcolor(&fb, x, y, get_ray_color(ray));
        }
    }

    fb_print_ppm(&fb);

    fb_free(&fb);
}
