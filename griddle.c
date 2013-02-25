/**
 * \mainpage
 *
 * This project attempts to recreate the semantics of the grid graphics package
 * for R as a C library using cairo.
 */

#include "griddle.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

/**
 * NOTE: this function assumes device coordinates are pixels.
 */
static double
unit_to_npc_helper(double dev_per_npc, double dev_per_line, double dev_per_em,
                   double o_ntv, double size_ntv, const unit_t *u)
{
    if (strcmp(u->type, "+") == 0) {
        double x = unit_to_npc_helper(dev_per_npc, dev_per_line, dev_per_em, 
                                      o_ntv, size_ntv, u->arg1);
        double y = unit_to_npc_helper(dev_per_npc, dev_per_line, dev_per_em, 
                                      o_ntv, size_ntv, u->arg2);
        return x + y;
    } else if (strcmp(u->type, "-") == 0) {
        double x = unit_to_npc_helper(dev_per_npc, dev_per_line, dev_per_em, 
                                      o_ntv, size_ntv, u->arg1);
        double y = unit_to_npc_helper(dev_per_npc, dev_per_line, dev_per_em, 
                                      o_ntv, size_ntv, u->arg2);
        return x - y;
    } else if (strcmp(u->type, "*") == 0) {
        double x = unit_to_npc_helper(dev_per_npc, dev_per_line, dev_per_em, 
                                      o_ntv, size_ntv, u->arg1);
        return x * u->value;
    } else if (strcmp(u->type, "/") == 0) {
        double x = unit_to_npc_helper(dev_per_npc, dev_per_line, dev_per_em, 
                                      o_ntv, size_ntv, u->arg1);
        return x / u->value;
    } else if (strcmp(u->type, "npc") == 0) {
        return u->value;
    } else if (strcmp(u->type, "px") == 0) {
        return u->value / dev_per_npc;
    } else if (strncmp(u->type, "line", 4) == 0) {
        return u->value * dev_per_line / dev_per_npc;
    } else if (strcmp(u->type, "em") == 0) {
        return u->value * dev_per_em / dev_per_npc;
    } else if (strcmp(u->type, "native") == 0) {
        return (u->value - o_ntv) / size_ntv;
    } else {
        fprintf(stderr, "Warning: can't convert unit '%s' to npc\n", u->type);
        return 0.0;
    }
}

/**
 * Convert a unit to a single NPC value.
 */
static double
unit_to_npc(grid_context_t *gr, char dim, const unit_t *u) {
    grid_viewport_node_t *node = gr->current_node;
    double dev_x_per_npc, dev_y_per_npc;
    dev_x_per_npc = dev_y_per_npc = 1.0;
    cairo_matrix_transform_distance(node->npc_to_dev, &dev_x_per_npc, 
                                                      &dev_y_per_npc);

    double x_ntv, y_ntv, w_ntv, h_ntv;
    x_ntv = y_ntv = 0.0;
    w_ntv = h_ntv = 1.0;
    cairo_matrix_transform_point(node->npc_to_ntv, &x_ntv, &y_ntv);
    cairo_matrix_transform_distance(node->npc_to_ntv, &w_ntv, &h_ntv);

    cairo_font_extents_t font_extents;
    cairo_font_extents(gr->cr, &font_extents);

    cairo_text_extents_t em_extents;
    cairo_text_extents(gr->cr, "m", &em_extents);

    double dev_per_npc, o_ntv, size_ntv;  
    dev_per_npc = o_ntv = size_ntv = 0.0;

    if (dim == 'x') {
        dev_per_npc = dev_x_per_npc;
        o_ntv = x_ntv;
        size_ntv = w_ntv;
    } else if (dim == 'y') {
        dev_per_npc = dev_y_per_npc;
        o_ntv = y_ntv;
        size_ntv = h_ntv;
    } else {
        fprintf(stderr, "Warning: unknown dimension '%c'\n", dim);
    }

    double result = unit_to_npc_helper(dev_per_npc, font_extents.height,
                                       em_extents.width, o_ntv, size_ntv, u);

    return result;
}

/**
 * NOTE: this function assumes device coordinates are pixels.
 */
static void
unit_array_to_npc_helper(double *result, double dev_per_npc, 
                         double dev_per_line, double dev_per_em, 
                         double o_ntv, double size_ntv,
                         int size, const unit_array_t *u) 
{
    int i;
    double *xs, *ys;

    if (strcmp(u->type, "+") == 0) {
        xs = malloc(size * sizeof(double));
        ys = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, dev_per_npc, dev_per_line, dev_per_em, 
                                 o_ntv, size_ntv, size, u->arg1);
        unit_array_to_npc_helper(ys, dev_per_npc, dev_per_line, dev_per_em, 
                                 o_ntv, size_ntv, size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] + ys[i];

        free(xs);
        free(ys);
    } else if (strcmp(u->type, "-") == 0) {
        xs = malloc(size * sizeof(double));
        ys = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, dev_per_npc, dev_per_line, dev_per_em, 
                                 o_ntv, size_ntv, size, u->arg1);
        unit_array_to_npc_helper(ys, dev_per_npc, dev_per_line, dev_per_em, 
                                 o_ntv, size_ntv, size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] - ys[i];

        free(xs);
        free(ys);
    } else if (strcmp(u->type, "*") == 0) {
        xs = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, dev_per_npc, dev_per_line, dev_per_em, 
                                 o_ntv, size_ntv, size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] * u->values[0];

        free(xs);
    } else if (strcmp(u->type, "/") == 0) {
        xs = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, dev_per_npc, dev_per_line, dev_per_em, 
                                 o_ntv, size_ntv, size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] * u->values[0];

        free(xs);
    } else if (strcmp(u->type, "npc") == 0) {
        for (i = 0; i < size; i++)
            result[i] = u->values[i];
    } else if (strcmp(u->type, "px") == 0) {
        for (i = 0; i < size; i++)
            result[i] = u->values[i] / dev_per_npc;
    } else if (strncmp(u->type, "line", 4) == 0) {
        for (i = 0; i < size; i++)
            result[i] = u->values[i] * dev_per_line / dev_per_npc;
    } else if (strcmp(u->type, "em") == 0) {
        for (i = 0; i < size; i++)
            result[i] = u->values[i] * dev_per_em / dev_per_npc;
    } else if (strcmp(u->type, "native") == 0) {
        for (i = 0; i < size; i++)
            result[i] = (u->values[i] - o_ntv) / size_ntv;
    } else {
        fprintf(stderr, "Warning: can't convert unit '%s' to npc\n", u->type);
        for (i = 0; i < size; i++)
            result[i] = 0.0;
    }
}

/**
 * Convert a unit array to a C array of doubles representing NPC values.
 */
static void
unit_array_to_npc(double *result, grid_context_t *gr, char dim, 
                  const unit_array_t *u) 
{
    grid_viewport_node_t *node = gr->current_node;
    double dev_x_per_npc, dev_y_per_npc;
    dev_x_per_npc = dev_y_per_npc = 1.0;
    cairo_matrix_transform_distance(node->npc_to_dev, &dev_x_per_npc, 
                                                      &dev_y_per_npc);

    double x_ntv, y_ntv, w_ntv, h_ntv;
    x_ntv = y_ntv = 0.0;
    w_ntv = h_ntv = 1.0;
    cairo_matrix_transform_point(node->npc_to_ntv, &x_ntv, &y_ntv);
    cairo_matrix_transform_distance(node->npc_to_ntv, &w_ntv, &h_ntv);

    cairo_font_extents_t font_extents;
    cairo_font_extents(gr->cr, &font_extents);

    cairo_text_extents_t em_extents;
    cairo_text_extents(gr->cr, "m", &em_extents);

    double dev_per_npc, o_ntv, size_ntv;
    dev_per_npc = o_ntv = size_ntv = 0.0;

    if (dim == 'x') {
        dev_per_npc = dev_x_per_npc;
        o_ntv = x_ntv;
        size_ntv = w_ntv;
    } else if (dim == 'y') {
        dev_per_npc = dev_y_per_npc;
        o_ntv = y_ntv;
        size_ntv = h_ntv;
    } else {
        fprintf(stderr, "Warning: unknown dimension '%c'\n", dim);
    }

    unit_array_to_npc_helper(result, dev_per_npc, font_extents.height,
                             em_extents.width, o_ntv, size_ntv, 
                             unit_array_size(u), u);
}

//
// graphics parameters
//

/**
 * Allocate an \ref rgba_t with the given red, green, and blue parameters.
 * `alpha` is set to 1 (opacity).
 */
rgba_t*
rgb(double red, double green, double blue) {
    rgba_t *color = malloc(sizeof(rgba_t));
    color->red = red;
    color->green = green;
    color->blue = blue;
    color->alpha = 1;

    return color;
}

/**
 * Allocate an \ref rgba_t with the given red, green, blue, and alpha parameters.
 */
rgba_t*
rgba(double red, double green, double blue, double alpha) {
    rgba_t *color = malloc(sizeof(rgba_t));
    color->red = red;
    color->green = green;
    color->blue = blue;
    color->alpha = alpha;

    return color;
}

/**
 * Allocate a new parameter struct with parameters set to default values.
 */
grid_par_t*
new_grid_default_par(void) {
    grid_par_t *par = malloc(sizeof(grid_par_t));

    par->color = rgba(0, 0, 0, 1);
    par->fill = NULL;

    char *line_type = "solid";
    par->line_type = malloc(strlen(line_type) + 1);
    strcpy(par->line_type, line_type);

    char *point_type = "round";
    par->point_type = malloc(strlen(point_type) + 1);
    strcpy(par->point_type, point_type);

    char *just = "center";
    par->just = malloc(strlen(just) + 1);
    strcpy(par->just, just);

    char *vjust = "top";
    par->vjust = malloc(strlen(vjust) + 1);
    strcpy(par->vjust, vjust);

    par->line_width = unit(2, "px");
    par->point_size = unit(4, "px");
    par->font_size = unit(20, "px");

    return par;
}

/**
 * Deallocate a parameter struct.
 */
void
free_grid_par(grid_par_t *par) {
    if (par->color)
        free(par->color);

    if (par->line_type)
        free(par->line_type);

    if (par->line_width)
        free(par->line_width);

    free(par);
}

//
// viewports
//

/**
 * Allocate a new \ref grid_viewport_t. By default, the native scale is NPC.
 *
 * \param x The x coordinate of the center of the new viewport.
 * \param y The y coordinate of the center of the new viewport.
 * \param width The width of the new viewport.
 * \param height The height of the new viewport.
 * \return A pointer to the newly allocated \ref grid_viewport_t.
 */
grid_viewport_t*
new_grid_viewport(unit_t *x, unit_t *y, unit_t *width, unit_t *height) {
    grid_viewport_t *vp = malloc(sizeof(grid_viewport_t));
    vp->x = x;
    vp->y = y;
    vp->w = width;
    vp->h = height;

    vp->has_ntv = false;

    return vp;
}

/**
 * Allocate a new \ref grid_viewport_t with default values. `x` and `y` are set
 * to 0 and `width` and `height` are set to 1. All units are npc.
 *
 * \return A pointer to the newly allocated \ref grid_viewport_t.
 */
grid_viewport_t*
new_grid_default_viewport() {
    return new_grid_viewport(unit(0, "npc"), unit(0, "npc"), 
                             unit(1, "npc"), unit(1, "npc"));
}

/**
 * Allocate a new \ref grid_viewport_t whose native coordinate system is
 * calculated from the given data.
 */
grid_viewport_t*
new_grid_data_viewport(int data_size, const double *xs, const double *ys) {
    grid_viewport_t *vp = new_grid_default_viewport();
    vp->has_ntv = true;

    if (data_size > 0) {
        double min, max, pad;
        min = max = xs[0];

        int i;
        for (i = 1; i < data_size; i++) {
            if (xs[i] > max)
                max = xs[i];
            else if (xs[i] < min)
                min = xs[i];
        }

        pad = 0.1 * (max - min) / 1.8;
        vp->x_ntv = min - pad;
        vp->w_ntv = max - min + 2 * pad;

        min = max = ys[0];
        for (i = 1; i < data_size; i++) {
            if (ys[i] > max)
                max = ys[i];
            else if (ys[i] < min)
                min = ys[i];
        }

        pad = 0.1 * (max - min) / 1.8;
        vp->y_ntv = min - pad;
        vp->h_ntv = max - min + 2 * pad;
    }

    return vp;
}

/**
 * Allocate a \ref grid_viewport_t with margins given by the arguments. Argument
 * values are interpreted as number of lines in the current font.
 */
grid_viewport_t*
new_grid_plot_viewport(grid_context_t *gr, 
                       double top, double right, double bottom, double left) 
{
    unit_t *x = unit(left, "lines");

    unit_t *y = unit(bottom, "lines");

    unit_t *width = unit_sub(unit(1, "npc"), 
                             unit_add(unit(left, "lines"), unit(right, "lines")));

    unit_t *height = unit_sub(unit(1, "npc"), 
                              unit_add(unit(top, "lines"), unit(bottom, "lines")));

    return new_grid_viewport(x, y, width, height);
}

/**
 * Deallocate a \ref grid_viewport_t.
 *
 * \param vp A viewport.
 */
void
free_grid_viewport(grid_viewport_t *vp) {
    if (vp->x)
        free_unit(vp->x);
    if (vp->y)
        free_unit(vp->y);
    if (vp->w)
        free_unit(vp->w);
    if (vp->h)
        free_unit(vp->h);

    free(vp);
}

/**
 * Allocate a new \ref grid_viewport_node_t.
 */
static grid_viewport_node_t*
new_grid_viewport_node(void) {
    grid_viewport_node_t *node = malloc(sizeof(grid_viewport_node_t));
    node->parent = node->gege = node->didi = node->child = NULL;
    node->name = NULL;
    node->npc_to_dev = malloc(sizeof(cairo_matrix_t));
    node->npc_to_ntv = malloc(sizeof(cairo_matrix_t));
    node->par = NULL;

    cairo_matrix_init_identity(node->npc_to_ntv);
    cairo_matrix_init_identity(node->npc_to_dev);

    return node;
}

/**
 * Deallocate a \ref grid_viewport_node_t.
 */
static void
free_grid_viewport_node(grid_viewport_node_t *node) {
    if (node->npc_to_ntv)
        free(node->npc_to_ntv);
    if (node->npc_to_dev)
        free(node->npc_to_dev);
    if (node->name)
        free(node->name);
    if (node->par)
        free(node->par);

    free(node);
}

void
grid_push_named_viewport(grid_context_t *gr, 
                         const char *name, const grid_viewport_t *vp)
{
    double x = unit_to_npc(gr, 'x', vp->x);
    double y = unit_to_npc(gr, 'y', vp->y);
    double w = unit_to_npc(gr, 'x', vp->w);
    double h = unit_to_npc(gr, 'y', vp->h);

    cairo_matrix_t vp_mtx, temp_mtx;
    cairo_matrix_init(&vp_mtx, w, 0, 0, h, x, y);
    cairo_matrix_init(&temp_mtx, w, 0, 0, h, x, y);
    cairo_status_t status = cairo_matrix_invert(&temp_mtx);

    if (status == CAIRO_STATUS_SUCCESS) {
        grid_viewport_node_t *node = new_grid_viewport_node();
        cairo_matrix_multiply(node->npc_to_dev, &vp_mtx, gr->current_node->npc_to_dev);

        if (vp->has_ntv) {
            cairo_matrix_init(node->npc_to_ntv, vp->w_ntv, 0, 0, vp->h_ntv, 
                                                vp->x_ntv, vp->y_ntv);
        } else {
            // inherit native coordinates from parent
            cairo_matrix_multiply(node->npc_to_ntv, 
                                  &vp_mtx, gr->current_node->npc_to_ntv);
        }

        if (name) {
            node->name = malloc(strlen(name) + 1);
            strcpy(node->name, name);
        }

        if (gr->current_node->child) {
            gr->current_node->child->didi = node;
            node->gege = gr->current_node->child;
        }

        gr->current_node->child = node;
        node->parent = gr->current_node;
        gr->current_node = node;
    } else {
        fprintf(stderr, "Warning: can't create singular viewport\n");
    }
}

/**
 * Push a viewport onto the tree. The viewport becomes a leaf of the current
 * viewport and becomes the new current viewport. This function acts
 * destructively on `vp->x`, `vp->y`, `vp->w`, and `vp->h`.
 */
void
grid_push_viewport(grid_context_t *gr, const grid_viewport_t *vp) {
    grid_push_named_viewport(gr, NULL, vp);
}

/**
 * Pop and deallocate the current viewport node from the tree; its parent
 * becomes the new current viewport.
 *
 * \return True if successful, false otherwise.
 */
bool
grid_pop_viewport_1(grid_context_t *gr) {
    if (gr->current_node == gr->root_node) {
        fprintf(stderr, "Warning: attempted to pop root viewport from the stack.\n");
        return false;
    } else {
        grid_viewport_node_t *node = gr->current_node;
        gr->current_node = node->parent;

        if (node->gege) {
            if (node->didi) {
                node->didi->gege = node->gege;
                node->gege->didi = node->didi;
                node->didi = node->gege = NULL;
            } else {
                node->parent->child = node->gege;
                node->gege->didi = NULL;
                node->gege = NULL;
            }
        }

        free_grid_viewport_node(node);
        return true;
    }
}

/**
 * Pop `n` viewports from the tree. The new current viewport is the parent of
 * the last popped viewport. If there are fewer than `n + 1` levels in the
 * tree, then an error message is printed and the current viepwort is set to the
 * root viewport. Popped viewports and the nodes that contain them are not
 * deallocated.
 *
 * \return The number of viewport successfully popped.
 */
int
grid_pop_viewport(grid_context_t *gr, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (!grid_pop_viewport_1(gr))
            // grid_up_viewport_1 will print an error message
            break;
    }

    return i;
}

/**
 * Move up one viewport in the viewport tree. Prints a warning and does nothing
 * if the current viewport is the root viewport.
 *
 * \return `true` if successful, `false` if the current viewport is the root
 * viewport.
 */
bool
grid_up_viewport_1(grid_context_t *gr) {
    if (gr->current_node == gr->root_node) {
        fprintf(stderr, "Warning: attempted to move up from root viewport\n");
        return false;
    }

    gr->current_node = gr->current_node->parent;
    return true;
}

/**
 * Move up `n` viewports in the viewport tree. Prints a warning if you try to
 * move beyond the root viewport.
 *
 * \return The number of viewports traversed.
 */
int
grid_up_viewport(grid_context_t *gr, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (!grid_up_viewport_1(gr))
            // grid_up_viewport_1 will print an error message
            break;
    }

    return i;
}

/**
 * Perform a depth first search for a viewport with the given name. `*plevel`
 * is incremented each time the algorithm recurses on a child viewport and is
 * set to -1 if no matching viewport is found. `path` is a list of nodes
 * traversed to find the target. If the return value is NULL, then the value of
 * `*path` is undefined and `*plevel` is set to -1.
 */
static grid_viewport_node_t*
grid_viewport_dfs(grid_viewport_node_t *this, const char *name, int *plevel) 
{
    if (this->name && strcmp(this->name, name) == 0) {
        return this;
    }
    
    grid_viewport_node_t *node;
    grid_viewport_node_t *this_child = this->child;
    int temp_level;

    while (this_child != NULL) {
        temp_level = *plevel + 1;
        node = grid_viewport_dfs(this_child, name, &temp_level);

        if (node) {
            *plevel = temp_level;
            return node;
        } else {
            this_child = this_child->gege;
        }
    }

    *plevel = -1;
    return NULL;
}

/**
 * Move down the tree to the viewport with the given name. Prints a warning if
 * no viewport is found. If the name matches the name of the current node, then
 * the current viewport doesn't change.
 *
 * \return The number of levels traversed, -1 if no viewport found.
 */
int
grid_down_viewport(grid_context_t *gr, const char *name) {
    int n = 0;
    grid_viewport_node_t *node = grid_viewport_dfs(gr->current_node, name, &n);

    if (node)
        gr->current_node = node;
    else
        fprintf(stderr, "Warning: didn't find viewport with name '%s'\n", name);

    return n;
}

/**
 * Perform a depth-first search for a viewport named `name` in the viewport
 * tree, starting from the root.  Prints a warning if no viewport is found.
 *
 * \return The new level in the viewport tree, -1 if no viewport found.
 */
int
grid_seek_viewport(grid_context_t *gr, const char *name) {
    int level = 0;
    grid_viewport_node_t *node = grid_viewport_dfs(gr->root_node, name, &level);

    if (node)
        gr->current_node = node;
    else
        fprintf(stderr, "Warning: didn't find viewport with name '%s'\n", name);

    return level;
}

//
// draw functions
//

static void
grid_apply_color(grid_context_t *gr, const rgba_t *color) {
    cairo_set_source_rgba(gr->cr, color->red, color->green, color->blue, color->alpha);
}

/**
 * Set the global (foreground) color.
 *
 * \return The old color.
 */
rgba_t*
grid_set_color(grid_context_t *gr, rgba_t *color) {
    rgba_t *old = gr->par->color;
    gr->par->color = color;
    grid_apply_color(gr, color);
    return old;
}

/**
 * Set the global fill color.
 *
 * \return The old color.
 */
rgba_t*
grid_set_fill(grid_context_t *gr, rgba_t *fill) {
    rgba_t *old = gr->par->color;
    gr->par->fill = fill;
    return old;
}

static double *grid_dash_pattern1_px = (double[]){10, 5};
static int grid_dash_pattern1_len = 2;

static void
grid_apply_line_type(grid_context_t *gr, const char *line_type) {
    if (strncmp(line_type, "dash", 4) == 0) {
        double dash_pattern1_dev[grid_dash_pattern1_len], temp;
        int i;
        unit_t this_unit;
        for (i = 0; i < grid_dash_pattern1_len; i++) {
            this_unit = Unit(grid_dash_pattern1_px[i], "px");
            dash_pattern1_dev[i] = unit_to_npc(gr, 'x', &this_unit);
            temp = 0;
            cairo_matrix_transform_distance(gr->current_node->npc_to_dev,
                                            dash_pattern1_dev + i, &temp);
        }
        cairo_set_dash(gr->cr, dash_pattern1_dev, grid_dash_pattern1_len, 0);
    } else {
        cairo_set_dash(gr->cr, NULL, 0, 0);
    }
}

/**
 * Set the global line type.
 */
char*
grid_set_line_type(grid_context_t *gr, char *line_type) {
    char *old = gr->par->line_type;
    gr->par->line_type = line_type;
    grid_apply_line_type(gr, line_type);
    return old;
}

/**
 * Set the global point type.
 */
char*
grid_set_point_type(grid_context_t *gr, char *pty) {
    char *old = gr->par->point_type;
    gr->par->point_type = pty;
    return old;
}

/**
 * Set the global horizontal justification.
 */
char*
grid_set_just(grid_context_t *gr, char *just) {
    char *old = gr->par->just;
    gr->par->just = just;
    return old;
}

/**
 * Set the global vertical justification.
 */
char*
grid_set_vjust(grid_context_t *gr, char *vjust) {
    char *old = gr->par->vjust;
    gr->par->vjust = vjust;
    return old;
}

static void
grid_apply_line_width(grid_context_t *gr, const unit_t *lwd) {
    double lwd_npc = unit_to_npc(gr, 'x', lwd);
    double temp = 0;
    cairo_matrix_transform_distance(gr->current_node->npc_to_dev, &lwd_npc, &temp);
    cairo_set_line_width(gr->cr, lwd_npc);
}

/**
 * Set the global line width to the given value.
 *
 * \return The old line width.
 */
unit_t*
grid_set_line_width(grid_context_t *gr, unit_t *lwd) {
    unit_t *old = gr->par->line_width;
    gr->par->line_width = lwd;
    grid_apply_line_width(gr, lwd);
    return old;
}

/**
 * Set the global point size to the given value.
 *
 * \return The old font size.
 */
unit_t*
grid_set_point_size(grid_context_t *gr, unit_t *point_size) {
    unit_t *old = gr->par->point_size;
    gr->par->point_size = point_size;
    return old;
}

static void
grid_apply_font_size(grid_context_t *gr, const unit_t *font_size) {
    double x_npc = unit_to_npc(gr, 'x', font_size);
    double temp = 0;
    cairo_matrix_transform_distance(gr->current_node->npc_to_dev, &x_npc, &temp);
    cairo_set_font_size(gr->cr, x_npc);
}

/**
 * Set the global font size to the given value.
 *
 * \return The old font size.
 */
unit_t*
grid_set_font_size(grid_context_t *gr, unit_t *font_size) {
    unit_t *old = gr->par->font_size;
    gr->par->font_size = font_size;
    grid_apply_font_size(gr, font_size);
    return old;
}

#define Parameter(F,A,B,C) (A && A->F ? A->F : B && B->F ? B->F : C->F)

/**
 * Attempt to set parameters first from the passed \ref grid_par_t, then from
 * the current node parameters, and finally from the global parameters. Sets the
 * cairo source color to the foreground color.
 */
static void
grid_apply_parameters(grid_context_t *gr, const grid_par_t *par) {
    rgba_t *col;
    char *s;
    unit_t *u;

    grid_par_t *cur = gr->current_node->par;
    grid_par_t *def = gr->par;

    col = Parameter(color, par, cur, def);
    grid_apply_color(gr, col);

    s = Parameter(line_type, par, cur, def);
    grid_apply_line_type(gr, s);

    u = Parameter(line_width, par, cur, def);
    grid_apply_line_width(gr, u);

    u = Parameter(font_size, par, cur, def);
    grid_apply_font_size(gr, u);
}

/**
 * For any non-null fields in `par`, set the corresponding value to the current
 * node parameters where applicable, and the global defaults otherwise.
 */
static void
grid_restore_parameters(grid_context_t *gr, const grid_par_t *par) {
    if (par) {
        rgba_t *col;
        char *s;
        unit_t *u;

        grid_par_t *nil = NULL;
        grid_par_t *cur = gr->current_node->par;
        grid_par_t *def = gr->par;

        if (par->color) {
            col = Parameter(color, nil, cur, def);
            grid_apply_color(gr, col);
        }

        if (par->line_type) {
            s = Parameter(line_type, nil, cur, def);
            grid_apply_line_type(gr, s);
        }

        if (par->line_width) {
            u = Parameter(line_width, nil, cur, def);
            grid_apply_line_width(gr, u);
        }

        if (par->font_size) {
            u = Parameter(font_size, nil, cur, def);
            grid_apply_font_size(gr, u);
        }
    }
}

/**
 * Allocate a new grid context. The grid context contains a reference to a cairo
 * image surface with the given width and height that it can draw to.
 *
 * \param width_px The width of the underlying image in pixels.
 * \param height_px The height of the underlying image in pixels.
 * \return A pointer to the newly allocated \ref grid_context_t.
 */
grid_context_t*
new_grid_context(int width_px, int height_px) {
    grid_context_t *gr = malloc(sizeof(grid_context_t));
    gr->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width_px, height_px);
    gr->cr = cairo_create(gr->surface);

    // put the origin at the lower left instead of the upper left
    cairo_matrix_t m = { .xx = 1, .yy = -1, .y0 = height_px };
    cairo_set_matrix(gr->cr, &m);

    grid_viewport_node_t *root = new_grid_viewport_node();
    root->name = malloc(sizeof(char) * 5);
    strcpy(root->name, "root");
    cairo_matrix_scale(root->npc_to_ntv, width_px, height_px);
    cairo_matrix_scale(root->npc_to_dev, width_px, height_px);
    gr->current_node = gr->root_node = root;

    grid_par_t *par = new_grid_default_par();
    gr->par = par;

    grid_apply_parameters(gr, NULL);

    return gr;
}

/**
 * Recursively deallocate a viewport tree and referenced viewports. The
 * implementation assumes the top-level root node does not have any siblings.
 *
 * \todo Current implementation is incomplete, it should free parameters.
 */
void
free_grid_viewport_tree(grid_viewport_node_t *root) {
    if (root->gege)
        free_grid_viewport_tree(root->gege);

    if (root->child)
        free_grid_viewport_tree(root->child);

    free_grid_viewport_node(root);
}

/**
 * Deallocate a \ref grid_context_t.
 */
void
free_grid_context(grid_context_t *gr) {
    free_grid_viewport_tree(gr->root_node);
    cairo_destroy(gr->cr);
    cairo_surface_destroy(gr->surface);
    free(gr);
}

/**
 * Draw a line connecting two points.
 */
void
grid_line(grid_context_t *gr, const unit_t *x1, const unit_t *y1, 
          const unit_t *x2, const unit_t *y2, const grid_par_t *par)
{
    grid_apply_parameters(gr, par);

    cairo_t *cr = gr->cr;
    double x1_npc = unit_to_npc(gr, 'x', x1);
    double y1_npc = unit_to_npc(gr, 'y', y1);
    double x2_npc = unit_to_npc(gr, 'x', x2);
    double y2_npc = unit_to_npc(gr, 'y', y2);

    cairo_matrix_t *m = gr->current_node->npc_to_dev;
    cairo_matrix_transform_point(m, &x1_npc, &y1_npc);
    cairo_matrix_transform_point(m, &x2_npc, &y2_npc);

    cairo_new_path(cr);
    cairo_move_to(cr, x1_npc, y1_npc);
    cairo_line_to(cr, x2_npc, y2_npc);

    cairo_stroke(cr);
    grid_restore_parameters(gr, par);
}

/**
 * Draw a line that connects the coordinates given by `xs` and `ys`.
 */
void
grid_lines(grid_context_t  *gr, const unit_array_t *xs, const unit_array_t *ys, 
           const grid_par_t *par) 
{
    grid_apply_parameters(gr, par);
    cairo_t *cr = gr->cr;
    
    int x_size = unit_array_size(xs);
    int y_size = unit_array_size(ys);

    if (x_size <= 0) {
        fprintf(stderr, "Warning: can't draw 0 length array.\n");
        return;
    } else if (x_size != y_size) {
        fprintf(stderr, "Warning: can't draw arrays of different sizes.\n");
        return;
    }

    double *xs_npc = malloc(x_size * sizeof(double));
    double *ys_npc = malloc(x_size * sizeof(double));

    unit_array_to_npc(xs_npc, gr, 'x', xs);
    unit_array_to_npc(ys_npc, gr, 'y', ys);

    cairo_matrix_t *m = gr->current_node->npc_to_dev;
    cairo_matrix_transform_point(m, xs_npc, ys_npc);
    cairo_new_path(cr);
    cairo_move_to(cr, xs_npc[0], ys_npc[0]);

    int i;
    for (i = 1; i < x_size; i++) {
        cairo_matrix_transform_point(m, xs_npc + i, ys_npc + i);
        cairo_line_to(cr, xs_npc[i], ys_npc[i]);
    }

    cairo_stroke(cr);
    grid_restore_parameters(gr, par);

    free(xs_npc);
    free(ys_npc);
}

/**
 * Draw a round point with the given size at the current location.
 */
static void
grid_point_round(grid_context_t *gr, double x_dev, double y_dev, double size_dev) {
    cairo_new_sub_path(gr->cr);
    cairo_arc(gr->cr, x_dev, y_dev, size_dev, 0, 2 * M_PI);
}

/**
 * Draw a point at the given coordinates.
 */
void
grid_point(grid_context_t *gr, const unit_t *x, const unit_t *y, 
           const grid_par_t *par) 
{
    grid_apply_parameters(gr, par);

    double x_npc = unit_to_npc(gr, 'x', x);
    double y_npc = unit_to_npc(gr, 'y', y);

    unit_t *psz = Parameter(point_size, par, gr->current_node->par, gr->par);
    double psz_npc = unit_to_npc(gr, 'x', psz);
    double temp = 0.0;

    cairo_matrix_t *m = gr->current_node->npc_to_dev;
    cairo_matrix_transform_point(m, &x_npc, &y_npc);
    cairo_matrix_transform_distance(m, &psz_npc, &temp);

    cairo_new_path(gr->cr);
    cairo_move_to(gr->cr, x_npc, y_npc);

    char *pty = Parameter(point_type, par, gr->current_node->par, gr->par);
    if (strcmp(pty, "round") == 0) {
        grid_point_round(gr, x_npc, y_npc, psz_npc);
    } else {
        fprintf(stderr, "Unknown point type: '%s'\n", pty);
        grid_point_round(gr, x_npc, y_npc, psz_npc);
    }

    cairo_fill(gr->cr);

    grid_restore_parameters(gr, par);
}

/**
 * Draw a point at each of the coordinates defined by `xs` and `ys`.
 */
void
grid_points(grid_context_t *gr, const unit_array_t *xs, const unit_array_t *ys,
            const grid_par_t *par)
{
    grid_apply_parameters(gr, par);

    int x_size = unit_array_size(xs);
    int y_size = unit_array_size(ys);

    if (x_size <= 0) {
        fprintf(stderr, "Warning: can't draw 0 length array.\n");
        return;
    } else if (x_size != y_size) {
        fprintf(stderr, "Warning: can't draw arrays of different sizes.\n");
        return;
    }

    double *xs_npc = malloc(x_size * sizeof(double));
    double *ys_npc = malloc(x_size * sizeof(double));

    unit_array_to_npc(xs_npc, gr, 'x', xs);
    unit_array_to_npc(ys_npc, gr, 'y', ys);

    unit_t *psz = Parameter(point_size, par, gr->current_node->par, gr->par);
    double psz_npc = unit_to_npc(gr, 'x', psz);
    double temp = 0.0;

    cairo_matrix_t *m = gr->current_node->npc_to_dev;
    cairo_matrix_transform_distance(m, &psz_npc, &temp);

    void (*draw_fn)(grid_context_t*, double, double, double);
    char *pty = Parameter(point_type, par, gr->current_node->par, gr->par);
    if (strcmp(pty, "round") == 0) {
        draw_fn = grid_point_round;
    } else {
        fprintf(stderr, "Unknown point type: '%s'\n", pty);
        draw_fn = grid_point_round;
    }

    cairo_new_path(gr->cr);

    int i;
    for (i = 0; i < x_size; i++) {
        cairo_matrix_transform_point(m, xs_npc + i, ys_npc + i);
        draw_fn(gr, xs_npc[i], ys_npc[i], psz_npc);
    }

    cairo_fill(gr->cr);
    grid_restore_parameters(gr, par);

    free(xs_npc);
    free(ys_npc);
}

/**
 * Draw a rectangle with lower-left corner at `(x, y)`.
 */
void
grid_rect(grid_context_t *gr, const unit_t *x, const unit_t *y, 
          const unit_t *width, const unit_t *height, const grid_par_t *par) 
{
    grid_apply_parameters(gr, par);

    double x_npc = unit_to_npc(gr, 'x', x);
    double y_npc = unit_to_npc(gr, 'y', y);
    double w_npc = unit_to_npc(gr, 'x', width);
    double h_npc = unit_to_npc(gr, 'y', height);

    cairo_matrix_t *m = gr->current_node->npc_to_dev;
    cairo_matrix_transform_point(m, &x_npc, &y_npc);
    cairo_matrix_transform_distance(m, &w_npc, &h_npc);

    cairo_new_path(gr->cr);
    cairo_rectangle(gr->cr, x_npc, y_npc, w_npc, h_npc);

    rgba_t *col;
    if ((par && (col = par->fill)) || 
        (gr->current_node->par && (col = gr->current_node->par->fill))) 
    {
        cairo_set_source_rgba(gr->cr, col->red, col->green,
                              col->blue, col->alpha);
        cairo_fill_preserve(gr->cr);

        col = Parameter(color, par, gr->current_node->par, gr->par);
        grid_apply_color(gr, col);
    }

    cairo_stroke(gr->cr);
    grid_restore_parameters(gr, par);
}

/**
 * Draw a rectangle that exactly occupies the current viewport. this command
 * can be used to fill backgrounds and draw borders on viewports. Equivalent
 * to 
 *
 *     grid_rect(gr, unit(0, "npc"), unit(0, "npc"), 
 *                   unit(1, "npc"), unit(1, "npc"), par);
 */
void
grid_full_rect(grid_context_t *gr, const grid_par_t *par) {
    unit_t zero = Unit(0, "npc");
    unit_t one = Unit(1, "npc");
    grid_rect(gr, &zero, &zero, &one, &one, par);
}

/**
 * Allocates a \ref unit_t indicating where to place the `x`-coordinate so that
 * text with width `width_npc` has the alignment given by `just`.
 */
static unit_t*
grid_just_to_x(char *just, const cairo_text_extents_t *extents) {
    unit_t *x;

    if (strncmp(just, "left", 1) == 0) {
        x = unit(1, "em");
    } else if (strncmp(just, "right", 1) == 0) {
        x = unit_sub(unit_sub(unit(1, "npc"), unit(1, "em")),
                     unit(extents->width + extents->x_bearing, "px"));
    } else if (strncmp(just, "center", 1) == 0) {
        x = unit_sub(unit(0.5, "npc"), 
                     unit(extents->width / 2.0 + extents->x_bearing, "px"));
    } else {
        fprintf(stderr, "Warning: unknown justification '%s'\n", just);
        x = unit(0, "npc");
    }

    return x;
}

/**
 * Allocates a \ref unit_t indicating where to place the `y`-coordinate so that
 * text has the vertical alignment given by `vjust`.
 */
static unit_t*
grid_vjust_to_y(char *vjust, const cairo_text_extents_t *extents) {
    unit_t *y;

    if (strncmp(vjust, "middle", 1) == 0) {
        // "add" instead of "sub" because y_bearing is measured with a downward
        // y-axis.
        y = unit_add(unit(0.5, "npc"), 
                     unit(extents->height / 2.0 + extents->y_bearing, "px"));
    } else if (strncmp(vjust, "bottom", 1) == 0) {
        y = unit(1, "line");
    } else if (strncmp(vjust, "top", 1) == 0) {
        y = unit_sub(unit(1, "npc"), unit(1, "line"));
    } else {
        fprintf(stderr, "Warning: unknown vertical justification '%s'\n", vjust);
        y = unit(0, "npc");
    }

    return y;
}

/**
 * Write the given text with lower-left corner of the displayed text at `(x, y)`.
 * See the parameter description for details.
 *
 * \param gr The grid context.
 * \param text The text to be displayed.
 * \param x The x-coordinate of the lower left corner of the displayed text.
 *   If `x` is `NULL`, set the horizontal alignment according to the value of
 *   `par->just` (falling back on the global default). `just` should be one of
 *   "left", "right", or "center".
 * \param y The y-coordinate of the lower left corner of the displayed text.
 *   If `y` is `NULL`, set the vertical alignment according to the value of
 *   `par->vjust` (falling back on the global default). `vjust` should be one of
 *   "top", "middle", and "bottom".
 * \param par Graphical parameters to apply to this drawing command.
 */
void
grid_text(grid_context_t *gr, const char *text, 
          const unit_t *x, const unit_t *y, const grid_par_t *par) 
{
    grid_apply_parameters(gr, par);

    cairo_t *cr = gr->cr;
    cairo_text_extents_t text_extents;
    cairo_text_extents(cr, text, &text_extents);

    unit_t *my_x = NULL; 

    if (!x) {
        if (par && par->just) 
            my_x = grid_just_to_x(par->just, &text_extents);
        else
            my_x = grid_just_to_x(gr->par->just, &text_extents);

        x = my_x;
    }

    unit_t *my_y = NULL;

    if (!y) {
        if (par && par->vjust) 
            my_y = grid_vjust_to_y(par->vjust, &text_extents);
        else
            my_y = grid_vjust_to_y(gr->par->vjust, &text_extents);

        y = my_y;
    }

    double x_npc = unit_to_npc(gr, 'x', x);
    double y_npc = unit_to_npc(gr, 'y', y);
    cairo_matrix_t *npc_to_dev = gr->current_node->npc_to_dev;
    cairo_matrix_transform_point(npc_to_dev, &x_npc, &y_npc);

    // unless we temporarily flip the coordinate system, cairo will draw the
    // text upside-down.

    cairo_matrix_t m;
    cairo_get_matrix(cr, &m);
    cairo_matrix_t id = { .xx = 1, .yy = 1 };
    cairo_set_matrix(cr, &id);

    cairo_new_path(cr);
    cairo_move_to(cr, x_npc, m.y0 - y_npc);
    cairo_show_text(cr, text);

    grid_restore_parameters(gr, par);
    cairo_set_matrix(cr, &m);

    if (my_x)
        free_unit(my_x);
    if (my_y)
        free_unit(my_y);
}

/**
 * Copy a format string to `fmt` appropriate to the scale. The last argument
 * is passed to `strncpy` and `snprintf`.
 */
static double
grid_scale_step_and_format(double scale, char *fmt, int n) {
    double step = pow(10, floor(scale));
    double intpart;
    double fracpart = modf(scale, &intpart);

    strcpy(fmt, "%.0f"); // default

    if (scale >= 0) {
        if (0.3 < fracpart && fracpart <= 0.5) {
            step *= 0.5;
            if (scale < 1)
                strncpy(fmt, "%.1f", n);
        } else if (fracpart <= 0.3) {
            step *= 0.25;
            if (scale < 1)
                strncpy(fmt, "%.2f", n);
            else if (scale < 2)
                strncpy(fmt, "%.1f", n);
        }
    } else {
        snprintf(fmt, n, "%%.%df", (int)floor(scale));
    }

    return step;
}

/**
 * Add labeled tick marks to the bottom of the current viewport. Tick location
 * and labels are generated from the viewport's native coordinate system.
 */
void
grid_xaxis(grid_context_t *gr, const grid_par_t *par) {
    grid_apply_parameters(gr, par);

    double x_ntv, y_ntv, w_ntv, h_ntv;
    x_ntv = y_ntv = 0.0;
    w_ntv = h_ntv = 1.0;
    cairo_matrix_transform_point(gr->current_node->npc_to_ntv, &x_ntv, &y_ntv);
    cairo_matrix_transform_distance(gr->current_node->npc_to_ntv, &w_ntv, &h_ntv);

    double scale = log10(w_ntv);
    char fmt[20];
    double step = grid_scale_step_and_format(scale, fmt, 19);

    // TODO do a similar adjustment for scale < 0?

    // find the smallest tick greater than x_ntv
    double first_tick = ceil(x_ntv / step) * step; 

    // count the number of ticks we can fit
    int n_ticks = 0;
    double t;
    for (t = first_tick; t < x_ntv + w_ntv; t += step)
        n_ticks++;

    double ticks[n_ticks];
    int i;
    for (i = 0; i < n_ticks; i++)
        ticks[i] = first_tick + i*step;

    unit_t height = Unit(0.4, "lines");
    double height_npc = unit_to_npc(gr, 'y', &height);

    unit_t x_unit = { .type = "native" };
    unit_t y_unit = Unit(-1.5, "line");
    double x1_npc, x2_npc, y1_npc, y2_npc;
    char buf[20];
    cairo_text_extents_t text_extents; 
    cairo_matrix_t m, id;
    id = (cairo_matrix_t){ .xx = 1, .yy = 1 };

    for (i = 0; i < n_ticks; i++) {
        x_unit.value = ticks[i];
        x1_npc = unit_to_npc(gr, 'x', &x_unit);
        y1_npc = 0;
        x2_npc = x1_npc;
        y2_npc = -height_npc;
        cairo_matrix_transform_point(gr->current_node->npc_to_dev, &x1_npc, &y1_npc);
        cairo_matrix_transform_point(gr->current_node->npc_to_dev, &x2_npc, &y2_npc);

        cairo_new_path(gr->cr);
        cairo_move_to(gr->cr, x1_npc, y1_npc);
        cairo_line_to(gr->cr, x2_npc, y2_npc);
        cairo_stroke(gr->cr);

        snprintf(buf, 20, fmt, ticks[i]);
        cairo_text_extents(gr->cr, buf, &text_extents);
        x1_npc = unit_to_npc(gr, 'x', &x_unit);
        y1_npc = unit_to_npc(gr, 'y', &y_unit);
        cairo_matrix_transform_point(gr->current_node->npc_to_dev, &x1_npc, &y1_npc);
        x1_npc -= text_extents.width / 2;

        cairo_get_matrix(gr->cr, &m);
        cairo_set_matrix(gr->cr, &id);
        cairo_new_path(gr->cr);
        cairo_move_to(gr->cr, x1_npc, m.y0 - y1_npc);
        cairo_show_text(gr->cr, buf);
        cairo_set_matrix(gr->cr, &m);
    }

    grid_restore_parameters(gr, par);
}

/**
 * Add labeled tick marks to the left side of the current viewport. Tick
 * location and labels are generated from the viewport's native coordinate
 * system.
 */
void
grid_yaxis(grid_context_t *gr, const grid_par_t *par) {
    grid_apply_parameters(gr, par);

    double x_ntv, y_ntv, w_ntv, h_ntv;
    x_ntv = y_ntv = 0.0;
    w_ntv = h_ntv = 1.0;
    cairo_matrix_transform_point(gr->current_node->npc_to_ntv, &x_ntv, &y_ntv);
    cairo_matrix_transform_distance(gr->current_node->npc_to_ntv, &w_ntv, &h_ntv);

    double scale = log10(h_ntv);
    char fmt[20];
    double step = grid_scale_step_and_format(scale, fmt, 19);

    // TODO do a similar adjustment for scale < 0?

    // find the smallest tick greater than x_ntv
    double first_tick = ceil(y_ntv / step) * step; 

    // count the number of ticks we can fit
    int n_ticks = 0;
    double t;
    for (t = first_tick; t < y_ntv + h_ntv; t += step)
        n_ticks++;

    double ticks[n_ticks];
    int i;
    for (i = 0; i < n_ticks; i++)
        ticks[i] = first_tick + i*step;

    unit_t width = Unit(0.75, "em");
    double width_npc = unit_to_npc(gr, 'x', &width);

    unit_t x_unit = Unit(-1.5, "em");
    unit_t y_unit = { .type = "native" };
    double x1_npc, x2_npc, y1_npc, y2_npc;
    char buf[20];
    cairo_text_extents_t text_extents; 
    cairo_matrix_t m, id;
    id = (cairo_matrix_t){ .xx = 1, .yy = 1 };

    for (i = 0; i < n_ticks; i++) {
        y_unit.value = ticks[i];
        x1_npc = 0;
        y1_npc = unit_to_npc(gr, 'y', &y_unit);
        x2_npc = -width_npc;
        y2_npc = y1_npc;
        cairo_matrix_transform_point(gr->current_node->npc_to_dev, &x1_npc, &y1_npc);
        cairo_matrix_transform_point(gr->current_node->npc_to_dev, &x2_npc, &y2_npc);

        cairo_new_path(gr->cr);
        cairo_move_to(gr->cr, x1_npc, y1_npc);
        cairo_line_to(gr->cr, x2_npc, y2_npc);
        cairo_stroke(gr->cr);

        snprintf(buf, 20, fmt, ticks[i]);
        cairo_text_extents(gr->cr, buf, &text_extents);
        x1_npc = unit_to_npc(gr, 'x', &x_unit);
        y1_npc = unit_to_npc(gr, 'y', &y_unit);
        cairo_matrix_transform_point(gr->current_node->npc_to_dev, &x1_npc, &y1_npc);
        x1_npc -= text_extents.width;
        y1_npc -= text_extents.height / 2;

        cairo_get_matrix(gr->cr, &m);
        cairo_set_matrix(gr->cr, &id);
        cairo_new_path(gr->cr);
        cairo_move_to(gr->cr, x1_npc, m.y0 - y1_npc);
        cairo_show_text(gr->cr, buf);
        cairo_set_matrix(gr->cr, &m);
    }

    grid_restore_parameters(gr, par);
}

