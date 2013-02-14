#include "griddle.h"
#include "grid_solarized.h"

#include <math.h>
#include <stdlib.h>

#define Pi 3.141592653589793

int
main(void) {
    grid_context_t *gr = new_grid_context(800, 600);

    grid_par_t par = {.color = &transparent, .fill = &lightbg1};
    grid_full_rect(gr, &par);

    grid_set_font_size(gr, unit(30, "px"));
    grid_push_viewport(gr,
      new_grid_viewport(unit(0, "npc"), unit_sub(unit(1, "npc"), unit(2, "lines")),
                        unit(1, "npc"), unit(2, "lines")));
    par = (grid_par_t){.color = &content1, .vjust = "middle"};
    grid_text(gr, "the sine function", NULL, NULL, &par);
    grid_pop_viewport_1(gr);

    double x[100], y[100];
    int i;
    for (i = 0; i < 100; i++) {
        x[i] = 2 * Pi * i / 100.0;
        y[i] = sin(x[i]);
    }

    grid_push_viewport(gr, new_grid_plot_viewport(gr, 2, 1, 1, 1));
    grid_push_viewport(gr, new_grid_data_viewport(100, x, y));
    grid_set_line_width(gr, unit(5, "px"));

    par = (grid_par_t){.fill = &bg2};
    grid_full_rect(gr, &par);

    unit_array_t x_units = UnitArray(100, x, "native");
    unit_array_t y_units = UnitArray(100, y, "native");
    par = (grid_par_t){.color = &blue};
    grid_lines(gr, &x_units, &y_units, &par);

    for (i = 0; i < 100; i++) {
        y[i] = sin(x[i] + Pi/4);
    }

    par = (grid_par_t){.color = &orange};
    grid_lines(gr, &x_units, &y_units, &par);

    for (i = 0; i < 100; i++) {
        y[i] = sin(x[i] + Pi / 2);
    }

    par = (grid_par_t){.color = &green};
    grid_lines(gr, &x_units, &y_units, &par);

    for (i = 0; i < 100; i++) {
        y[i] = sin(x[i] + 3 * Pi / 4);
    }

    par = (grid_par_t){.color = &violet};
    grid_lines(gr, &x_units, &y_units, &par);

    cairo_surface_write_to_png(gr->surface, "sine.png");
}
