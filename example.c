#include "griddle.h"

#include <stdio.h>

int
main(void) {
    // cairo_surface_t *surface = 
    //     cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 800, 800);
    // cairo_t *cr = cairo_create(surface);

    // cairo_scale(cr, 400, 400);
    // cairo_rectangle(cr, 0.25, 0.25, 0.5, 0.5);
    // cairo_set_source_rgb(cr, 0, 1, 0);
    // cairo_fill(cr);

    // cairo_destroy(cr);
    // cairo_surface_write_to_png(surface, "hello.png");
    // cairo_surface_destroy(surface);

    grid_context_t *gr = new_grid_context(800, 800);
    grid_viewport_t *vp = new_grid_viewport(unit(0.25, "npc"), unit(0.25, "npc"),
                                            unit(0.5, "npc"), unit(0.5, "npc"),
                                            unit(0, "radian"));
    grid_push_viewport(gr, vp);

    grid_par_t *par = new_grid_par();
    par->green = 1;
    grid_line(gr, unit(0, "npc"), unit(1, "npc"), 
                  unit(0.5, "npc"), unit(0.5, "npc"), par);

    cairo_surface_write_to_png(gr->surface, "griddle.png");
}
