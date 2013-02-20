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

/**
 * NOTE: this function assumes device coordinates are pixels.
 */
static double
unit_to_npc_helper(double device_per_npc, double line_per_npc, double em_per_npc,
                   double o_ntv, double size_ntv, const unit_t *u)
{
    if (strcmp(u->type, "+") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_per_npc, em_per_npc, 
                                      o_ntv, size_ntv, u->arg1);
        double y = unit_to_npc_helper(device_per_npc, line_per_npc, em_per_npc, 
                                      o_ntv, size_ntv, u->arg2);
        return x + y;
    } else if (strcmp(u->type, "-") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_per_npc, em_per_npc, 
                                      o_ntv, size_ntv, u->arg1);
        double y = unit_to_npc_helper(device_per_npc, line_per_npc, em_per_npc, 
                                      o_ntv, size_ntv, u->arg2);
        return x - y;
    } else if (strcmp(u->type, "*") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_per_npc, em_per_npc, 
                                      o_ntv, size_ntv, u->arg1);
        return x * u->value;
    } else if (strcmp(u->type, "/") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_per_npc, em_per_npc, 
                                      o_ntv, size_ntv, u->arg1);
        return x / u->value;
    } else if (strcmp(u->type, "npc") == 0) {
        return u->value;
    } else if (strcmp(u->type, "px") == 0) {
        return u->value / device_per_npc;
    } else if (strncmp(u->type, "line", 4) == 0) {
        return u->value * line_per_npc;
    } else if (strcmp(u->type, "em") == 0) {
        return u->value * em_per_npc;
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
    double device_x_per_npc, device_y_per_npc;
    device_x_per_npc = device_y_per_npc = 1.0;
    cairo_matrix_transform_distance(node->npc_to_dev, &device_x_per_npc, 
                                                      &device_y_per_npc);

    double x_ntv, y_ntv, w_ntv, h_ntv;
    x_ntv = y_ntv = 0.0;
    w_ntv = h_ntv = 1.0;
    cairo_matrix_transform_point(node->npc_to_ntv, &x_ntv, &y_ntv);
    cairo_matrix_transform_distance(node->npc_to_ntv, &w_ntv, &h_ntv);

    cairo_font_extents_t *font_extents = malloc(sizeof(cairo_font_extents_t));
    cairo_font_extents(gr->cr, font_extents);

    cairo_text_extents_t *em_extents = malloc(sizeof(cairo_text_extents_t));
    cairo_text_extents(gr->cr, "m", em_extents);

    double device_per_npc, o_ntv, size_ntv;  
    device_per_npc = o_ntv = size_ntv = 0.0;

    if (dim == 'x') {
        device_per_npc = device_x_per_npc;
        o_ntv = x_ntv;
        size_ntv = w_ntv;
    } else if (dim == 'y') {
        device_per_npc = device_y_per_npc;
        o_ntv = y_ntv;
        size_ntv = h_ntv;
    } else {
        fprintf(stderr, "Warning: unknown dimension '%c'\n", dim);
    }

    double result = unit_to_npc_helper(device_per_npc, font_extents->height,
                                       em_extents->width, o_ntv, size_ntv, u);

    free(font_extents);
    free(em_extents);
    return result;
}

static void
unit_array_to_npc_helper(double *result, double device_per_npc, 
                         double line_per_npc, double em_per_npc, 
                         double o_ntv, double size_ntv,
                         int size, const unit_array_t *u) 
{
    int i;
    double *xs, *ys;

    if (strcmp(u->type, "+") == 0) {
        xs = malloc(size * sizeof(double));
        ys = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, device_per_npc, line_per_npc, em_per_npc, 
                                 o_ntv, size_ntv, size, u->arg1);
        unit_array_to_npc_helper(ys, device_per_npc, line_per_npc, em_per_npc, 
                                 o_ntv, size_ntv, size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] + ys[i];

        free(xs);
        free(ys);
    } else if (strcmp(u->type, "-") == 0) {
        xs = malloc(size * sizeof(double));
        ys = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, device_per_npc, line_per_npc, em_per_npc, 
                                 o_ntv, size_ntv, size, u->arg1);
        unit_array_to_npc_helper(ys, device_per_npc, line_per_npc, em_per_npc, 
                                 o_ntv, size_ntv, size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] - ys[i];

        free(xs);
        free(ys);
    } else if (strcmp(u->type, "*") == 0) {
        xs = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, device_per_npc, line_per_npc, em_per_npc, 
                                 o_ntv, size_ntv, size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] * u->values[0];

        free(xs);
    } else if (strcmp(u->type, "/") == 0) {
        xs = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, device_per_npc, line_per_npc, em_per_npc, 
                                 o_ntv, size_ntv, size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] * u->values[0];

        free(xs);
    } else if (strcmp(u->type, "npc") == 0) {
        for (i = 0; i < size; i++)
            result[i] = u->values[i];
    } else if (strcmp(u->type, "px") == 0) {
        for (i = 0; i < size; i++)
            result[i] = u->values[i] / device_per_npc;
    } else if (strncmp(u->type, "line", 4) == 0) {
        for (i = 0; i < size; i++)
            result[i] = u->values[i] / line_per_npc;
    } else if (strcmp(u->type, "em") == 0) {
        for (i = 0; i < size; i++)
            result[i] = u->values[i] * em_per_npc;
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
    double device_x_per_npc, device_y_per_npc;
    device_x_per_npc = device_y_per_npc = 1.0;
    cairo_matrix_transform_distance(node->npc_to_dev, &device_x_per_npc, 
                                                      &device_y_per_npc);

    double x_ntv, y_ntv, w_ntv, h_ntv;
    x_ntv = y_ntv = 0.0;
    w_ntv = h_ntv = 1.0;
    cairo_matrix_transform_point(node->npc_to_ntv, &x_ntv, &y_ntv);
    cairo_matrix_transform_distance(node->npc_to_ntv, &w_ntv, &h_ntv);

    cairo_font_extents_t *font_extents = malloc(sizeof(cairo_font_extents_t));
    cairo_font_extents(gr->cr, font_extents);

    cairo_text_extents_t *em_extents = malloc(sizeof(cairo_text_extents_t));
    cairo_text_extents(gr->cr, "m", em_extents);

    double device_per_npc, o_ntv, size_ntv;
    device_per_npc = o_ntv = size_ntv = 0.0;

    if (dim == 'x') {
        device_per_npc = device_x_per_npc;
        o_ntv = x_ntv;
        size_ntv = w_ntv;
    } else if (dim == 'y') {
        device_per_npc = device_y_per_npc;
        o_ntv = y_ntv;
        size_ntv = h_ntv;
    } else {
        fprintf(stderr, "Warning: unknown dimension '%c'\n", dim);
    }

    unit_array_to_npc_helper(result, device_per_npc, font_extents->height,
                             em_extents->width, o_ntv, size_ntv, 
                             unit_array_size(u), u);

    free(font_extents);
    free(em_extents);
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

    char *lty = "solid";
    par->lty = malloc(strlen(lty) + 1);
    strcpy(par->lty, lty);

    char *just = "center";
    par->just = malloc(strlen(just) + 1);
    strcpy(par->just, just);

    char *vjust = "top";
    par->vjust = malloc(strlen(vjust) + 1);
    strcpy(par->vjust, vjust);

    par->lwd = unit(2, "px");
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

    if (par->lty)
        free(par->lty);

    if (par->lwd)
        free(par->lwd);

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

    vp->x_ntv = vp->y_ntv = 0; 
    vp->w_ntv = vp->h_ntv = 1;

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
    node->npc_to_ntv = malloc(sizeof(cairo_matrix_t));
    node->npc_to_dev = malloc(sizeof(cairo_matrix_t));
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
        cairo_matrix_multiply(node->npc_to_ntv, &vp_mtx, gr->current_node->npc_to_ntv);
        cairo_matrix_multiply(node->npc_to_dev, &vp_mtx, gr->current_node->npc_to_dev);

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

static void
grid_apply_line_width(grid_context_t *gr, unit_t *lwd) {
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
    unit_t *old = gr->par->lwd;
    gr->par->lwd = lwd;
    grid_apply_line_width(gr, lwd);
    return old;
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

    // set the font size and line width (defaults should be in pixels)
    grid_apply_font_size(gr, par->font_size);
    grid_apply_line_width(gr, par->lwd);

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

static double *grid_dash_pattern1_px = (double[]){10, 5};
static int grid_dash_pattern1_len = 2;

/**
 * Apply the graphical parameters, fall back to defaults where applicable.
 */
static void
grid_apply_line_parameters(grid_context_t *gr, const grid_par_t *par, 
                           grid_par_t *default_par) 
{
    cairo_t *cr = gr->cr;
    rgba_t *color;
    if (!(par && (color = par->color)))
        color = default_par->color;

    cairo_set_source_rgba(cr, color->red, color->green, 
                          color->blue, color->alpha);

    char *lty;
    if (!(par && (lty = par->lty)))
        lty = default_par->lty;

    if (strncmp(lty, "dash", 4) == 0) {
        double dash_pattern1_npc[grid_dash_pattern1_len];
        int i;
        unit_t this_unit;
        for (i = 0; i < grid_dash_pattern1_len; i++) {
            this_unit = Unit(grid_dash_pattern1_px[i], "px");
            dash_pattern1_npc[i] = unit_to_npc(gr, 'x', &this_unit);
        }
        cairo_set_dash(cr, dash_pattern1_npc, grid_dash_pattern1_len, 0);
    } else {
        cairo_set_dash(cr, NULL, 0, 0);
    }

    unit_t *lwd;
    if (!(par && (lwd = par->lwd)))
        lwd = default_par->lwd;

    grid_apply_line_width(gr, lwd);
}

/**
 * Draw a line connecting two points.
 */
void
grid_line(grid_context_t *gr, const unit_t *x1, const unit_t *y1, 
          const unit_t *x2, const unit_t *y2, const grid_par_t *par)
{
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

    grid_apply_line_parameters(gr, par, gr->par);
    cairo_stroke(cr);
}

void
grid_lines(grid_context_t  *gr, const unit_array_t *xs, const unit_array_t *ys, 
           const grid_par_t *par) 
{
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

    grid_apply_line_parameters(gr, par, gr->par);
    cairo_stroke(cr);

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
    cairo_t *cr = gr->cr;
    double x_npc = unit_to_npc(gr, 'x', x);
    double y_npc = unit_to_npc(gr, 'y', y);
    double w_npc = unit_to_npc(gr, 'x', width);
    double h_npc = unit_to_npc(gr, 'y', height);

    cairo_matrix_t *m = gr->current_node->npc_to_dev;
    cairo_matrix_transform_point(m, &x_npc, &y_npc);
    cairo_matrix_transform_distance(m, &w_npc, &h_npc);

    cairo_new_path(cr);
    cairo_rectangle(cr, x_npc, y_npc, w_npc, h_npc);

    rgba_t *fill;
    if ((par && (fill = par->fill)) || (fill = gr->par->fill)) {
        cairo_set_source_rgba(cr, fill->red, fill->green,
                              fill->blue, fill->alpha);
        cairo_fill_preserve(cr);
    }

    grid_apply_line_parameters(gr, par, gr->par);
    cairo_stroke(cr);
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
 * Returns a \ref unit_t indicating where to place the `x`-coordinate so that
 * text with width `width_npc` has the alignment given by `just`.
 */
static unit_t*
grid_just_to_x(char *just, double width_npc) {
    unit_t *x;

    if (strncmp(just, "left", 1) == 0) {
        x = unit(1, "em");
    } else if (strncmp(just, "right", 1) == 0) {
        x = unit_sub(unit_sub(unit(1, "npc"), unit(1, "em")),
                     unit(width_npc, "npc"));
    } else if (strncmp(just, "center", 1) == 0) {
        x = unit_sub(unit(0.5, "npc"), unit(width_npc/2.0, "npc"));
    } else {
        fprintf(stderr, "Warning: unknown justification '%s'\n", just);
        x = unit(0, "npc");
    }

    return x;
}

/**
 * Returns a \ref unit_t indicating where to place the `y`-coordinate so that
 * text has the vertical alignment given by `vjust`.
 */
static unit_t*
grid_vjust_to_y(char *vjust) {
    unit_t *y;

    if (strncmp(vjust, "middle", 1) == 0) {
        y = unit_sub(unit(0.5, "npc"), unit(0.5, "line"));
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

    // set the font size first so that subsequent distance calculations are
    // correct.
    unit_t *font_size;
    if (par && par->font_size)
        font_size = par->font_size;
    else
        font_size = gr->par->font_size;

    grid_apply_font_size(gr, font_size);

    cairo_t *cr = gr->cr;
    cairo_text_extents_t *text_extents = malloc(sizeof(cairo_text_extents_t));
    cairo_text_extents(cr, text, text_extents);

    unit_t *my_x = NULL; 

    if (!x) {
        double width_npc = text_extents->width / gr->current_node->npc_to_dev->xx;

        if (par && par->just) 
            my_x = grid_just_to_x(par->just, width_npc);
        else
            my_x = grid_just_to_x(gr->par->just, width_npc);

        x = my_x;
    }

    unit_t *my_y = NULL;

    if (!y) {
        if (par && par->vjust) 
            my_y = grid_vjust_to_y(par->vjust);
        else
            my_y = grid_vjust_to_y(gr->par->vjust);

        y = my_y;
    }

    rgba_t *color;
    if (par && par->color)
        color = par->color;
    else
        color = gr->par->color;

    cairo_set_source_rgba(cr, color->red, color->blue, 
                          color->green, color->alpha);
    
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

    cairo_set_matrix(cr, &m);

    if (my_x)
        free_unit(my_x);
    if (my_y)
        free_unit(my_y);
    free(text_extents);
}

//// /**
////  * Draw tick marks along the x-axis at the specified values.
////  *
////  * \todo Labeled ticks.
////  */
//// void
//// grid_xaxis(grid_context_t *gr, const unit_array_t *at, const grid_par_t *par) {
////     grid_apply_line_parameters(gr, par, gr->par);
//// 
////     unit_t th = Unit(0.5, "lines");
////     double tick_height = unit_to_npc(gr, 'y', &th);
//// 
////     int i;
////     double npc;
////     unit_t u;
////     for (i = 0; i < at->size; i++) {
////         u = Unit(at->values[i], at->type);
////         npc = unit_to_npc(gr, 'x', &u);
//// 
////         cairo_new_path(gr->cr);
////         cairo_move_to(gr->cr, npc, 1);
////         cairo_line_to(gr->cr, npc, 1 + tick_height);
////         cairo_stroke(gr->cr);
////     }
//// }
