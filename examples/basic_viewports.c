#include "griddle.h"
#include "griddle_solarized.h"

#include <stdio.h>

int
main(void) {
    grid_context_t *gr = new_grid_context(800, 800);

    unit_t lwd = Unit(10, "px");
    grid_par_t par = {.lwd = &lwd};

    grid_rect(gr, unit(0, "npc"), unit(0, "npc"), 
                  unit(1, "npc"), unit(1, "npc"), &par);

    grid_viewport_t *vp1 = new_grid_named_viewport("vp1",
                             unit(0, "npc"), unit(0.5, "npc"),
                             unit(0.5, "npc"), unit(0.5, "npc"));
    grid_viewport_t *vp2 = new_grid_viewport(
                             unit(0.5, "npc"), unit(0, "npc"),
                             unit(0.5, "npc"), unit(0.5, "npc"));

    grid_push_viewport(gr, vp1);
    unit_t font_size = Unit(18, "px");
    par = (grid_par_t){.color = &content4, .fill = &lightbg1, 
                       .font_size = &font_size};
    grid_rect(gr, unit(0, "npc"), unit(0, "npc"), 
                  unit(1, "npc"), unit(1, "npc"), &par);
    par.color = &content1;
    grid_text(gr, "Some drawing in graphics region 1.",
              unit(0, "npc"), unit(0.8, "npc"), &par);

    grid_up_viewport_1(gr);
    grid_push_viewport(gr, vp2);
    par.color = &content4;
    grid_rect(gr, unit(0, "npc"), unit(0, "npc"), 
                  unit(1, "npc"), unit(1, "npc"), &par);
    par.color = &content1;
    grid_text(gr, "Some drawing in graphics region 2.",
              unit(0, "npc"), unit(0.8, "npc"), &par);

    grid_up_viewport_1(gr);
    grid_down_viewport(gr, "vp1");
    grid_text(gr, "Some more drawing in graphics region 1.",
              unit_add(unit(0, "npc"), unit(1, "em")), unit(0.2, "npc"), &par);

    cairo_surface_write_to_png(gr->surface, "basic_viewports.png");
}
