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
    char *line_type, *point_type, *just, *vjust;
    unit_t *line_width, *point_size, *font_size;
} grid_par_t;

/**
 * A viewport defines a relative coordinate system for drawing on the image
 * buffer.
 */
typedef struct {
    unit_t *x, *y, *w, *h;

    bool has_ntv;
    double x_ntv, y_ntv, w_ntv, h_ntv;
} grid_viewport_t;

/**
 * Nodes are viewports that have been captured in the viewport tree.
 */
typedef struct __grid_viewport_node_t {
    struct __grid_viewport_node_t *parent, 
                                  *gege,   /**< Older sibling ('gege' is a 
                                                romanization of the Chinese word 
                                                for older brother). */
                                  *didi,   /**< Younger sibling ('didi' is a
                                                romanization of the Chinese word
                                                for younger brother). */
                                  *child;

    char *name;
    cairo_matrix_t *npc_to_ntv, *npc_to_dev;
    grid_par_t *par;
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
grid_push_named_viewport(grid_context_t*, const char*, const grid_viewport_t*);

void
free_grid_viewport(grid_viewport_t*);

void
grid_push_viewport(grid_context_t*, const grid_viewport_t*);

bool
grid_pop_viewport_1(grid_context_t*);

int
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

rgba_t*
grid_set_color(grid_context_t*, rgba_t*);

rgba_t*
grid_set_fill(grid_context_t*, rgba_t*);

char*
grid_set_line_type(grid_context_t*, char*);

char*
grid_set_just(grid_context_t*, char*);

char*
grid_set_vjust(grid_context_t*, char*);

unit_t*
grid_set_line_width(grid_context_t*, unit_t*);

unit_t*
grid_set_font_size(grid_context_t*, unit_t*);

grid_context_t*
new_grid_context(int, int);

void
free_grid_viewport_tree(grid_viewport_node_t*);

void
free_grid_context(grid_context_t*);

void
grid_line(grid_context_t*, const unit_t*, const unit_t*, 
          const unit_t*, const unit_t*, const grid_par_t*);

void
grid_lines(grid_context_t*, const unit_array_t*, const unit_array_t*, 
           const grid_par_t*);

void
grid_point(grid_context_t*, const unit_t*, const unit_t*, const grid_par_t*);

void
grid_points(grid_context_t*, const unit_array_t*, const unit_array_t*,
            const grid_par_t*);

void
grid_rect(grid_context_t*, const unit_t*, const unit_t*, 
          const unit_t*, const unit_t*, const grid_par_t*);

void
grid_full_rect(grid_context_t*, const grid_par_t*);

void
grid_text(grid_context_t*, const char*, const unit_t*, const unit_t*, 
          const grid_par_t*);

void
grid_xaxis(grid_context_t*, const grid_par_t*);

void
grid_yaxis(grid_context_t*, const grid_par_t*);

#endif
