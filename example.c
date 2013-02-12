#include "griddle.h"

#include <stdio.h>

int
main(void) {
    grid_context_t *gr = new_grid_context(800, 800);
    grid_viewport_t *vp = new_grid_viewport(unit(0.25, "npc"), unit(0.25, "npc"),
                                            unit(0.5, "npc"), unit(0.5, "npc"));
    grid_push_viewport(gr, vp);

    grid_par_t *par;
    rgba_t *white = rgb(1, 1, 1);
    par = new_grid_par((grid_par_t){.color = white, .fill = white});
    grid_rect(gr, unit(0, "npc"), unit(0, "npc"), 
                  unit(1, "npc"), unit(1, "npc"), par);
    free_grid_par(par);

    rgba_t *green = rgb(0, 1, 0);
    par = new_grid_par((grid_par_t){.color = green});

    grid_line(gr, unit(0, "npc"), unit(1, "npc"), 
                  unit(0, "npc"), unit(1, "npc"), par);

    cairo_surface_write_to_png(gr->surface, "griddle.png");
}
