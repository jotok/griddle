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
unit_to_npc_helper(double device_per_npc, double line_per_npc, 
                   double em_per_npc, unit_t *u)
{
    if (strcmp(u->type, "+") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_per_npc, 
                                      em_per_npc, u->arg1);
        double y = unit_to_npc_helper(device_per_npc, line_per_npc, 
                                      em_per_npc, u->arg2);
        return x + y;
    } else if (strcmp(u->type, "-") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_per_npc, 
                                      em_per_npc, u->arg1);
        double y = unit_to_npc_helper(device_per_npc, line_per_npc, 
                                      em_per_npc, u->arg2);
        return x - y;
    } else if (strcmp(u->type, "*") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_per_npc, 
                                      em_per_npc, u->arg1);
        return x * u->value;
    } else if (strcmp(u->type, "/") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_per_npc, 
                                      em_per_npc, u->arg1);
        return x / u->value;
    } else if (strcmp(u->type, "npc") == 0) {
        return u->value;
    } else if (strcmp(u->type, "px") == 0) {
        return u->value / device_per_npc;
    } else if (strncmp(u->type, "line", 4) == 0) {
        return u->value * line_per_npc;
    } else if (strcmp(u->type, "em") == 0) {
        return u->value * em_per_npc;
    } else {
        fprintf(stderr, "Warning: can't convert unit '%s' to npc\n", u->type);
        return 0.0;
    }
}

/**
 * Convert a unit to a single NPC value.
 */
static double
unit_to_npc(cairo_t *cr, char dim, unit_t *u) {
    double device_x_per_npc = 1.0;
    double device_y_per_npc = 1.0;
    cairo_user_to_device_distance(cr, &device_x_per_npc, &device_y_per_npc);

    cairo_font_extents_t *font_extents = malloc(sizeof(cairo_font_extents_t));
    cairo_font_extents(cr, font_extents);

    cairo_text_extents_t *em_extents = malloc(sizeof(cairo_text_extents_t));
    cairo_text_extents(cr, "m", em_extents);

    double device_per_npc = 0.0;

    if (dim == 'x') {
        device_per_npc = device_x_per_npc;
    } else if (dim == 'y') {
        device_per_npc = device_y_per_npc;
    } else {
        fprintf(stderr, "Warning: unknown dimension '%c'\n", dim);
    }

    double result = unit_to_npc_helper(device_per_npc, font_extents->height,
                                       em_extents->width, u);
    free(font_extents);
    free(em_extents);
    return result;
}

static void
unit_array_to_npc_helper(double *result, double device_per_npc, 
                         double line_per_npc, double em_per_npc, 
                         int size, unit_array_t *u) 
{
    int i;
    double *xs, *ys;

    if (strcmp(u->type, "+") == 0) {
        xs = malloc(size * sizeof(double));
        ys = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, device_per_npc, line_per_npc, em_per_npc, 
                                 size, u->arg1);
        unit_array_to_npc_helper(ys, device_per_npc, line_per_npc, em_per_npc, 
                                 size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] + ys[i];

        free(xs);
        free(ys);
    } else if (strcmp(u->type, "-") == 0) {
        xs = malloc(size * sizeof(double));
        ys = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, device_per_npc, line_per_npc, em_per_npc, 
                                 size, u->arg1);
        unit_array_to_npc_helper(ys, device_per_npc, line_per_npc, em_per_npc, 
                                 size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] - ys[i];

        free(xs);
        free(ys);
    } else if (strcmp(u->type, "*") == 0) {
        xs = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, device_per_npc, line_per_npc, em_per_npc, 
                                 size, u->arg1);

        for (i = 0; i < size; i++)
            result[i] = xs[i] * u->values[0];

        free(xs);
    } else if (strcmp(u->type, "/") == 0) {
        xs = malloc(size * sizeof(double));
        unit_array_to_npc_helper(xs, device_per_npc, line_per_npc, em_per_npc, 
                                 size, u->arg1);

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
unit_array_to_npc(double *result, cairo_t *cr, char dim, unit_array_t *u) {
    double device_x_per_npc = 1.0;
    double device_y_per_npc = 1.0;
    cairo_user_to_device_distance(cr, &device_x_per_npc, &device_y_per_npc);

    cairo_font_extents_t *font_extents = malloc(sizeof(cairo_font_extents_t));
    cairo_font_extents(cr, font_extents);

    cairo_text_extents_t *em_extents = malloc(sizeof(cairo_text_extents_t));
    cairo_text_extents(cr, "m", em_extents);

    double device_per_npc = 0.0;

    if (dim == 'x') {
        device_per_npc = device_x_per_npc;
    } else if (dim == 'y') {
        device_per_npc = device_y_per_npc;
    } else {
        fprintf(stderr, "Warning: unknown dimension '%c'\n", dim);
    }

    unit_array_to_npc_helper(result, device_per_npc, font_extents->height,
                             em_extents->width, unit_array_size(u), u);

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
 * Allocate a new \ref grid_viewport_t.
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
    vp->width = width;
    vp->height = height;
    vp->name = NULL;

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
 * Allocate a new \ref grid_viewport_t with the given name. 
 *
 * \return A pointer to the newly allocated \ref grid_viewport_t.
 * \see new_grid_viewport
 */
grid_viewport_t*
new_grid_named_viewport(const char *name, unit_t *x, unit_t *y, 
                        unit_t *width, unit_t *height) 
{
    grid_viewport_t *vp = new_grid_viewport(x, y, width, height);
    vp->name = malloc(strlen(name) + 1);
    strcpy(vp->name, name);
    return vp;
}

/**
 * Allocate a new \ref grid_viewport_t with the given name and default values.
 *
 * \return A pointer to the newly allocated \ref grid_viewport_t.
 * \see new_grid_named_viewport
 */
grid_viewport_t*
new_grid_named_default_viewport(const char *name) {
    grid_viewport_t *vp = new_grid_default_viewport();
    vp->name = malloc(strlen(name) + 1);
    strcpy(vp->name, name);
    return vp;
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
    if (vp->width)
        free_unit(vp->width);
    if (vp->height)
        free_unit(vp->height);

    free(vp->par);
    free(vp);
}

/**
 * Allocate a new \ref grid_viewport_node_t.
 */
static grid_viewport_node_t*
new_grid_viewport_node(grid_viewport_t *vp) {
    grid_viewport_node_t *node = malloc(sizeof(grid_viewport_node_t));
    node->vp = vp;
    node->parent = node->gege = node->didi = node->child = NULL;

    return node;
}

/**
 * Apply the transform described by `vp` to the cairo context.
 */
static void
grid_apply_viewport_transform(grid_context_t *gr, grid_viewport_t *vp) {
    cairo_save(gr->cr);

    double height_npc = unit_to_npc(gr->cr, 'y', vp->height);

    cairo_translate(gr->cr, unit_to_npc(gr->cr, 'x', vp->x),
                            1 - unit_to_npc(gr->cr, 'y', vp->y) - height_npc);
    cairo_scale(gr->cr, unit_to_npc(gr->cr, 'x', vp->width), height_npc);
}

/**
 * Push a viewport onto the tree. The viewport becomes a leaf of the current
 * viewport and becomes the new current viewport.
 */
void
grid_push_viewport(grid_context_t *gr, grid_viewport_t *vp) {
    grid_viewport_node_t *node = new_grid_viewport_node(vp);

    if (gr->current_node->child) {
        gr->current_node->child->didi = node;
        node->gege = gr->current_node->child;
    }

    gr->current_node->child = node;
    node->parent = gr->current_node;
    gr->current_node = node;

    grid_apply_viewport_transform(gr, vp);
}

/**
 * Pop the current viewport node from the tree; its parent becomes the new
 * current viewport. The popped node is not deallocated since it may contain
 * viewports still in use in its tree structure.
 *
 * \return A pointer to the popped viewport.
 */
grid_viewport_node_t*
grid_pop_viewport_1(grid_context_t *gr) {
    grid_viewport_node_t *node = NULL;

    if (gr->current_node == gr->root_node) {
        fprintf(stderr, "Warning: attempted to pop root viewport from the stack in %s:%d\n",
                __FILE__, __LINE__);
    } else {
        cairo_restore(gr->cr);

        node = gr->current_node;
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
    }

    return node;
}

/**
 * Pop `n` viewports from the tree. The new current viewport is the parent of
 * the last popped viewport. If there are fewer than `n + 1` levels in the
 * tree, then an error message is printed and the current viepwort is set to the
 * root viewport. Popped viewports and the nodes that contain them are not
 * deallocated.
 *
 * \return A pointer to a node pointing to the last popped viewport.
 */
grid_viewport_node_t*
grid_pop_viewport(grid_context_t *gr, int n) {
    grid_viewport_node_t *node, *try_node;
    node = try_node = NULL;

    int i;
    for (i = 0; i < n; i++) {
        if ((try_node = grid_pop_viewport_1(gr)) != NULL)
            node = try_node;
        else
            // grid_up_viewport_1 will print an error message
            break;
    }

    return node;
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
        fprintf(stderr, "Warning: attempted to move up from root viewport in %s:%d\n",
                __FILE__, __LINE__);
        return false;
    }

    cairo_restore(gr->cr);
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
 * A container for a linked list.
 */
typedef struct {
    grid_viewport_node_t *first, /**< Head of the list */
                         *last;  /**< Last member of the list */
} grid_viewport_list_t;

/**
 * Allocate a new \ref grid_viewport_list_t.
 */
static grid_viewport_list_t*
new_grid_viewport_list(grid_viewport_t *vp) {
    grid_viewport_node_t *node = new_grid_viewport_node(vp);
    grid_viewport_list_t *list = malloc(sizeof(grid_viewport_list_t));
    list->first = node;
    list->last = node;

    return list;
}

/**
 * Deallocate a \ref grid_viewport_list_t. This frees the nodes but the the
 * referenced viewports.
 */
static void
free_grid_viewport_list(grid_viewport_list_t *list) {
    grid_viewport_node_t *node, *next_node;
    node = list->first;

    while (node != list->last) {
        next_node = node->child;
        free(node);
        node = next_node;
    }

    free(list);
}

/**
 * Append a viewport to the end of the list.
 */
static void
grid_viewport_list_append(grid_viewport_list_t *list, grid_viewport_t *vp) {
    grid_viewport_node_t *node = new_grid_viewport_node(vp);
    list->last->child = node;
    list->last = node;
}

/**
 * Append all the nodes in `list2` to `list1`.
 */
static void
grid_viewport_list_concat(grid_viewport_list_t *list1, grid_viewport_list_t *list2) {
    list1->last->child = list2->first;
   list1->last = list2->last;
}

/**
 * Perform a depth first search for a viewport with the given name. `*plevel`
 * is incremented each time the algorithm recurses on a child viewport and is
 * set to -1 if no matching viewport is found. `path` is a list of nodes
 * traversed to find the target. If the return value is NULL, then the value of
 * `*path` is undefined and `*plevel` is set to -1.
 */
static grid_viewport_node_t*
grid_viewport_dfs(grid_viewport_node_t *this, const char *name, 
                  int *plevel, grid_viewport_list_t *path) 
{
    if (this->vp->name && strcmp(this->vp->name, name) == 0) {
        grid_viewport_list_append(path, this->vp);
        return this;
    }
    
    grid_viewport_node_t *node;
    grid_viewport_node_t *this_child = this->child;
    int temp_level;
    grid_viewport_list_t *temp_path;

    while (this_child != NULL) {
        temp_level = *plevel + 1;
        temp_path = new_grid_viewport_list(this->vp);
        node = grid_viewport_dfs(this_child, name, &temp_level, temp_path);

        if (node) {
            *plevel = temp_level;
            grid_viewport_list_concat(path, temp_path);
            free(temp_path); // but don't free the nodes within
            return node;
        } else {
            free_grid_viewport_list(temp_path);
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
    grid_viewport_list_t *path = new_grid_viewport_list(gr->current_node->vp);
    grid_viewport_node_t *node = grid_viewport_dfs(gr->current_node, name, &n, path);

    if (node) {
        grid_viewport_node_t *this = path->first->child;
        while (this) {
            grid_apply_viewport_transform(gr, this->vp);
            this = this->child;
        }

        gr->current_node = node;
    } else {
        fprintf(stderr, "Warning: didn't find viewport with name '%s' (%s:%d)\n",
                name, __FILE__, __LINE__);
    }

    free_grid_viewport_list(path);
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
    grid_viewport_list_t *path = new_grid_viewport_list(gr->current_node->vp);
    grid_viewport_node_t *node = grid_viewport_dfs(gr->root_node, name, &level, path);

    if (node) {
        int i;
        for (i = 0; i < level; i++)
            cairo_restore(gr->cr);

        grid_viewport_node_t *this = path->first->child;
        while (this) {
            grid_apply_viewport_transform(gr, this->vp);
            this = this->child;
        }

        gr->current_node = node;
    } else {
        fprintf(stderr, "Warning: didn't find viewport with name '%s' (%s:%d)\n",
                name, __FILE__, __LINE__);
    }

    free_grid_viewport_list(path);
    return level;
}

//
// draw functions
//

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

    // Scale so that user coordinates are NPC
    cairo_scale(gr->cr, width_px, height_px);

    grid_viewport_t *root = new_grid_named_default_viewport("root");
    gr->root_node = new_grid_viewport_node(root);
    gr->current_node = gr->root_node;

    gr->par = new_grid_default_par();

    return gr;
}

/**
 * Recursively deallocate a viewport tree and referenced viewports. The
 * implementation assumes the top-level root node does not have any siblings.
 */
void
free_grid_viewport_tree(grid_viewport_node_t *root) {
    if (root->gege)
        free_grid_viewport_tree(root->gege);

    if (root->child)
        free_grid_viewport_tree(root->child);

    free(root);
}

/**
 * Deallocate a \ref grid_context_t. Does not deallocate viewports.
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
grid_apply_line_parameters(cairo_t *cr, grid_par_t *par, grid_par_t *default_par) 
{
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
            dash_pattern1_npc[i] = unit_to_npc(cr, 'x', &this_unit);
        }
        cairo_set_dash(cr, dash_pattern1_npc, grid_dash_pattern1_len, 0);
    } else {
        cairo_set_dash(cr, NULL, 0, 0);
    }

    unit_t *lwd;
    if (!(par && (lwd = par->lwd)))
        lwd = default_par->lwd;

    cairo_set_line_width(cr, unit_to_npc(cr, 'x', lwd));
}

/**
 * Draw a line connecting two points.
 */
void
grid_line(grid_context_t *gr, unit_t *x1, unit_t *y1, unit_t *x2, unit_t *y2,
          grid_par_t *par)
{
    cairo_t *cr = gr->cr;
    double x1_npc = unit_to_npc(cr, 'x', x1);
    double y1_npc = unit_to_npc(cr, 'y', y1);
    double x2_npc = unit_to_npc(cr, 'x', x2);
    double y2_npc = unit_to_npc(cr, 'y', y2);

    cairo_new_path(cr);
    cairo_move_to(cr, x1_npc, 1 - y1_npc);
    cairo_line_to(cr, x2_npc, 1 - y2_npc);

    grid_apply_line_parameters(cr, par, gr->par);
    cairo_stroke(cr);
}

void
grid_lines(grid_context_t  *gr, unit_array_t *xs, unit_array_t *ys, grid_par_t *par) {
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

    unit_array_to_npc(xs_npc, cr, 'x', xs);
    unit_array_to_npc(ys_npc, cr, 'y', ys);

    cairo_new_path(cr);
    cairo_move_to(cr, xs_npc[0], 1 - ys_npc[0]);

    int i;
    for (i = 1; i < x_size; i++)
        cairo_line_to(cr, xs_npc[i], 1 - ys_npc[i]);

    grid_apply_line_parameters(cr, par, gr->par);
    cairo_stroke(cr);

    free(xs_npc);
    free(ys_npc);
}

/**
 * Draw a rectangle with lower-left corner at `(x, y)`.
 */
void
grid_rect(grid_context_t *gr, unit_t *x, unit_t *y, 
          unit_t *width, unit_t *height, grid_par_t *par) 
{
    cairo_t *cr = gr->cr;
    double x_npc = unit_to_npc(cr, 'x', x);
    double y_npc = unit_to_npc(cr, 'y', y);
    double width_npc = unit_to_npc(cr, 'x', width);
    double height_npc = unit_to_npc(cr, 'y', height);

    cairo_new_path(cr);
    cairo_rectangle(cr, x_npc, 1 - y_npc - height_npc, width_npc, height_npc);

    rgba_t *fill;
    if ((par && (fill = par->fill)) || (fill = gr->par->fill)) {
        cairo_set_source_rgba(cr, fill->red, fill->green,
                              fill->blue, fill->alpha);
        cairo_fill_preserve(cr);
    }

    grid_apply_line_parameters(cr, par, gr->par);
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
grid_full_rect(grid_context_t *gr, grid_par_t *par) {
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
        y = unit(0.5, "npc");
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
grid_text(grid_context_t *gr, const char *text, unit_t *x, unit_t *y,
          grid_par_t *par) 
{

    // set the font size first so that subsequent distance calculations are
    // correct.
    unit_t *font_size;
    if (par && par->font_size)
        font_size = par->font_size;
    else
        font_size = gr->par->font_size;

    cairo_t *cr = gr->cr;
    cairo_set_font_size(cr, unit_to_npc(cr, 'x', font_size));

    cairo_text_extents_t *text_extents = malloc(sizeof(cairo_text_extents_t));
    cairo_text_extents(cr, text, text_extents);

    unit_t *my_x = NULL; 

    if (!x) {
        if (par && par->just) 
            my_x = grid_just_to_x(par->just, text_extents->width);
        else
            my_x = grid_just_to_x(gr->par->just, text_extents->width);

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

    cairo_new_path(cr);
    cairo_move_to(cr, unit_to_npc(cr, 'x', x), 1 - unit_to_npc(cr, 'y', y));
    cairo_set_source_rgba(cr, color->red, color->blue, 
                          color->green, color->alpha);
    cairo_show_text(cr, text);

    if (my_x)
        free_unit(my_x);
    if (my_y)
        free_unit(my_y);
    free(text_extents);
}
