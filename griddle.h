#ifndef Griddle_h
#define Griddle_h

#include <stdbool.h>
#include <cairo.h>

#define GridLongNameLength 30
#define GridShortNameLength 10

// types

/**
 * A value tagged with a unit type. Using units we can specify "5 pixels + 2
 * lines" and have our program do the right thing.
 */
typedef struct __unit_t {
    double value;
    char type[GridShortNameLength];
    struct __unit_t *arg1, *arg2;
} unit_t;

/**
 * Graphical parameters.
 */
typedef struct {
    double red, green, blue, alpha;
    char lty[GridShortNameLength];
} grid_par_t;

/**
 * A viewport defines a relative coordinate system for drawing on the image
 * buffer.
 */
typedef struct {
    unit_t *x, *y, *width, *height, *angle;
    grid_par_t *par;
    char name[GridLongNameLength];
} grid_viewport_t;

/**
 * Nodes wrap viewports to keep track of the viewport tree structure.
 */
typedef struct __grid_viewport_node_t {
    grid_viewport_t *vp;
    struct __grid_viewport_node_t *parent, 
                                  *gege,   /**< Older sibling ('gege' is a 
                                                romanization of the Chinese word 
                                                for older brother). */
                                  *didi,   /**< Younger sibling ('didi' is a
                                                romanization of the Chinese word
                                                for younger brother). */
                                  *child;
} grid_viewport_node_t;

/**
 * A grid context consists of the viewport tree, the current viewport, and
 * cairo objects used to create the drawing.
 */
typedef struct {
    cairo_surface_t *surface;
    cairo_t *cr;
    grid_viewport_node_t *root_node, *current_node;
    grid_par_t *par;
} grid_context_t;

// units

unit_t*
unit(double, const char*);

unit_t*
unit_add(unit_t*, unit_t*);

unit_t*
unit_sub(unit_t*, unit_t*);

unit_t*
unit_mul(unit_t*, double);

unit_t*
unit_div(unit_t*, double);

void
free_unit(unit_t*, bool);

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
free_grid_viewport(grid_viewport_t*, bool);

void
grid_push_viewport(grid_context_t*, grid_viewport_t*);

grid_viewport_node_t*
grid_pop_viewport_1(grid_context_t*);

grid_viewport_node_t*
grid_pop_viewport(grid_context_t*, int);

bool
grid_up_viewport_1(grid_context_t*);

int
grid_up_viewport(grid_context_t*, int);

int
grid_down_viewport(grid_context_t*, const char*);

int
grid_seek_viewport(grid_context_t*, const char*);

// draw functions

grid_context_t*
new_grid_context(int, int);

void
free_grid_viewport_tree(grid_viewport_node_t*);

void
free_grid_context(grid_context_t*);

void
grid_line(grid_context_t*, unit_t*, unit_t*, unit_t*, unit_t*, grid_par_t*);

#endif
