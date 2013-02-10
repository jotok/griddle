#ifndef Griddle_h
#define Griddle_h

#include <stdbool.h>
#include <cairo.h>

#define GridLongNameLength 30
#define GridShortNameLength 10

// types

typedef struct __unit_t {
    double value;
    char type[GridShortNameLength];
    struct __unit_t *arg1, *arg2;
} unit_t;

typedef struct {
    char lty[GridShortNameLength];
} grid_par_t;

typedef struct {
    unit_t *x, *y, *width, *height, *angle;
    grid_par_t *par;
    char name[GridLongNameLength];
} grid_viewport_t;

typedef struct __grid_viewport_node_t {
    grid_viewport_t *vp;
    struct __grid_viewport_node_t *parent, *sibling, *child;
} grid_viewport_node_t;

typedef struct {
    cairo_surface_t *surface;
    cairo_t *cr;
    grid_viewport_node_t *root_node, *current_node;
} grid_context_t;

// units

unit_t*
unit(double, const char*);

unit_t*
unit_add(unit_t*, unit_t*);

unit_t*
unit_sub(unit_t*, unit_t*);

unit_t*
unit_mul(const unit_t*, double);

unit_t*
unit_div(const unit_t*, double);

// graphics parameters

grid_par_t*
new_grid_par(void);

void
grid_par_set_str(char*, const char*);

// viewports

grid_viewport_t*
new_grid_viewport(unit_t*, unit_t*, unit_t*, unit_t*, unit_t*);

grid_viewport_t*
new_grid_default_viewport(void);

grid_viewport_t*
new_grid_named_viewport(const char*, unit_t*, unit_t*, unit_t*, unit_t*, unit_t*);

grid_viewport_t*
new_grid_named_default_viewport(const char*);

void
grid_push_viewport(grid_context_t*, grid_viewport_t*);

grid_viewport_t*
grid_pop_viewport_1(grid_context_t*);

grid_viewport_t*
grid_pop_viewport(grid_context_t*, int);

bool
grid_up_viewport_1(grid_context_t*);

int
grid_up_viewport(grid_context_t*, int);

int
grid_down_viewport(grid_context_t*, const char *name);

int
grid_seek_viewport(grid_context_t*, const char *name);

// draw functions

grid_context_t*
new_grid_context(int h_px, int w_px);

#endif
