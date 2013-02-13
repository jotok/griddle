#include "griddle.h"
#include "griddle_solarized.h"

int
main(void) {
    grid_context_t *gr = new_grid_context(800, 800);
    grid_par_t par = {};
    unit_t x, y, width, height;

    width = height = Unit(200, "px");

    x = Unit(0, "px");
    y = Unit(0, "px");
    par.fill = &bg1;
    par.color = &transparent;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(200, "px");
    par.fill = &bg2;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(400, "px");
    par.fill = &content1;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(600, "px");
    par.fill = &content2;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(0, "px");
    y = Unit(200, "px");
    par.fill = &content3;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(200, "px");
    par.fill = &content4;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(400, "px");
    par.fill = &lightbg1;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(600, "px");
    par.fill = &lightbg2;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(0, "px");
    y = Unit(400, "px");
    par.fill = &yellow;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(200, "px");
    par.fill = &orange;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(400, "px");
    par.fill = &red;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(600, "px");
    par.fill = &magenta;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(0, "px");
    y = Unit(600, "px");
    par.fill = &violet;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(200, "px");
    par.fill = &blue;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(400, "px");
    par.fill = &cyan;
    grid_rect(gr, &x, &y, &width, &height, &par);

    x = Unit(600, "px");
    par.fill = &green;
    grid_rect(gr, &x, &y, &width, &height, &par);

    cairo_surface_write_to_png(gr->surface, "color_test.png");
}
