#include <stdlib.h>
#include <stdio.h>

#include "fb.h"
#include "vecmat/vec3f.h"

int main(void)
{
    const int fb_w = 320;
    const int fb_h = 240;
    int x, y;
    struct fb fb;

    fb_init(&fb, fb_w, fb_h);
    
    for (y = 0; y < fb_h; ++y) {
        for (x = 0; x < fb_w; ++x) {
            fb_putpixel(&fb, x, y, (float)x / fb_w, (float)y / fb_h, 0.2f,
                        1.0f);
        }
    }

    fb_print_ppm(&fb);

    fb_free(&fb);
}
