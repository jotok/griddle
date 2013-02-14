#ifndef Griddle_h
#define Griddle_h

#include "grid_units.h"

#include <stdbool.h>
#include <cairo.h>

// types

/**
 * A struct representing an RGBA value. All parameters should take values between
 * 0 and 1. For alpha, 1 indicates opacity and 0 indicates transparency.
 */
typedef struct {
    double red, green, blue, alpha;
} rgba_t;

/**
 * Graphical parameters.
 */
typedef struct {
    rgba_t *color, *fill;
    char *lty, *just, *vjust;
    unit_t *lwd, *font_size;
} grid_par_t;

/**
 * A viewport defines a relative coordinate system for drawing on the image
 * buffer.
 */
typedef struct {
    unit_t *x, *y, *width, *height;
    grid_par_t *par;
    char *name;

    bool has_ntv;
    double x_ntv, y_ntv, width_ntv, height_ntv;
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

// graphics parameters

/**
 * Construct an rgba_t literal with the given values and alpha = 1.
 */
#define RGB(R,G,B) ((rgba_t){.red = R, .green = G, .blue = B, .alpha = 1})

/**
 * Construct an rgba_t literal with the given values.
 */
#define RGBA(R,G,B,A) ((rgba_t){ .red = R, .green = G, .blue = B, .alpha = A })

/**
 * Construct an rgba_t literal with a grayscale value between 0 and 1.
 */
#define Grayscale(S) RGB(S,S,S)

rgba_t*
rgb(double, double, double);

rgba_t*
rgba(double, double, double, double);

grid_par_t*
new_grid_default_par(void);

void
free_grid_par(grid_par_t*);

// viewports

grid_viewport_t*
new_grid_viewport(unit_t*, unit_t*, unit_t*, unit_t*);

grid_viewport_t*
new_grid_default_viewport(void);

grid_viewport_t*
new_grid_named_viewport(const char*, unit_t*, unit_t*, unit_t*, unit_t*);

grid_viewport_t*
new_grid_named_default_viewport(const char*);

grid_viewport_t*
new_grid_data_viewport(int, const double*, const double*);

grid_viewport_t*
new_grid_plot_viewport(grid_context_t*, double, double, double, double);

void
free_grid_viewport(grid_viewport_t*);

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

void
grid_set_font_size(grid_context_t*, unit_t*);

void
grid_set_line_width(grid_context_t*, unit_t*);

grid_context_t*
new_grid_context(int, int);

void
free_grid_viewport_tree(grid_viewport_node_t*);

void
free_grid_context(grid_context_t*);

void
grid_line(grid_context_t*, unit_t*, unit_t*, unit_t*, unit_t*, grid_par_t*);

void
grid_lines(grid_context_t*, unit_array_t*, unit_array_t*, grid_par_t*);

void
grid_rect(grid_context_t*, unit_t*, unit_t*, unit_t*, unit_t*, grid_par_t*);

void
grid_full_rect(grid_context_t*, grid_par_t*);

void
grid_text(grid_context_t*, const char*, unit_t*, unit_t*, grid_par_t*);

void
grid_xaxis(grid_context_t*, const unit_array_t*, grid_par_t*);

#endif
