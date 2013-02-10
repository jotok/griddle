#include "griddle.h"

#include <stdio.h>

int
main(void) {
    cairo_surface_t *surface = 
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 800, 800);
    cairo_t *cr = cairo_create(surface);

    cairo_scale(cr, 400, 400);
    cairo_rectangle(cr, 0.25, 0.25, 0.5, 0.5);
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_fill(cr);

    cairo_destroy(cr);
    cairo_surface_write_to_png(surface, "hello.png");
    cairo_surface_destroy(surface);

    return 0;
}
