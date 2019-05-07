#ifndef FB_H
#define FB_H

struct color {
    float r, g, b, a;
};

struct fb {
    struct color *b;
    int w;
    int h;
};

#define DEFINE_FB(name, width, height)                  \
    struct color fb_data_##name[width * height];        \
    struct fb name = { fb_data_##name, width, height };

static void fb_init(struct fb *fb, int w, int h)
{
    fb->b = malloc(w * h * sizeof(fb->b[0]));
    fb->w = w;
    fb->h = h;
}

static void fb_free(struct fb *fb)
{
    free(fb->b);
    fb->b = NULL;
}

#ifdef FB_DO_BOUNDS_CHECK
static inline bool fb_out_of_bounds(struct fb *fb, int x, int y)
{
    return x < 0 || y < 0 || x >= fb->w || y >= fb->h;
}
#else
static inline bool fb_out_of_bounds(struct fb *fb, int x, int y)
{
    return false;
}
#endif

static inline void fb_putcolor(struct fb *fb, int x, int y, struct color c)
{
    if (fb_out_of_bounds(fb, x, y))
        return;

    fb->b[x + y * fb->w] = c;
}

static inline void fb_putpixel(struct fb *fb, int x, int y, float r, float g,
                               float b, float a)
{
    if (fb_out_of_bounds(fb, x, y))
        return;

    struct color c = { r, g, b, a };
    fb_putcolor(fb, x, y, c);
}

static inline struct color fb_getcolor(struct fb *fb, int x, int y)
{
    if (fb_out_of_bounds(fb, x, y))
        return (struct color) { -1.0f, -1.0f, -1.0f -1.0f };

    return fb->b[x + y * fb->w];
}

static void fb_print_ppm(struct fb *fb)
{
    int x, y;

    printf("P3\n%d %d\n255\n", fb->w, fb->h);
    for (y = fb->h - 1; y >= 0; --y) {
        for (x = 0; x < fb->w; ++x) {
            struct color c = fb_getcolor(fb, x, y);
            printf("%d %d %d\n", (int)(c.r * 255),
                                 (int)(c.g * 255),
                                 (int)(c.b * 255));
        }
    }
}

#endif
