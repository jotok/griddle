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

#define Pi 3.141593

//
// units
//

/**
 * Allocate a new \ref unit_t with the given type. Currently supported unit types
 * are "px" and "line".
 *
 * \return A pointer to the allocated `unit_t`.
 */
unit_t*
unit(double value, const char *type) {
    unit_t *u = malloc(sizeof(unit_t));
    u->value = value;
    strncpy(u->type, type, GridShortNameLength - 1);
    u->type[GridShortNameLength - 1] = '\0';

    return u;
}

/**
 * Allocate a new \ref unit_t representing the sum of its arguments.
 *
 * \return A unit representing arg1 + arg2.
 */
unit_t*
unit_add(unit_t *arg1, unit_t *arg2) {
    unit_t *u = unit(0.0, "+");
    u->arg1 = arg1;
    u->arg2 = arg2;

    return u;
}

/**
 * Allocate a new \ref unit_t representing the difference of its arguments.
 *
 * \return A unit representing arg1 - arg2.
 */
unit_t*
unit_sub(unit_t *arg1, unit_t *arg2) {
    unit_t *u = unit(0.0, "-");
    u->arg1 = arg1;
    u->arg2 = arg2;

    return u;
}

/**
 * Allocate a new \ref unit_t representing the value of `u` multiplied by
 * a scalar.
 * 
 * \return A unit representing u * x.
 */
unit_t*
unit_mul(unit_t* u, double x) {
    unit_t *v = unit(x, "*");
    v->arg1 = u;

    return v;
}

/**
 * Allocate a new \ref unit_t representing the value of `u` divided by
 * a scalar.
 * 
 * \return A unit representing u / x.
 */
unit_t*
unit_div(unit_t* u, double x) {
    unit_t *v = unit(x, "/");
    v->arg1 = u;

    return v;
}

/**
 * Deallocate a \ref unit_t.
 *
 * \param u A allocated unit.
 * \param free_recursive If true, then this call will recursively free units
 *   referenced from this unit.
 */
void
free_unit(unit_t *u, bool free_recursive) {
    if (free_recursive) {
        if (u->arg1)
            free_unit(u->arg1, true);
        if (u->arg2)
            free_unit(u->arg1, true);
    }

    free(u);
}

/**
 * Convert a \ref unit_t to a double representing the value of the unit in
 * radians. The types of all units in the tree should be one of "radian[s]",
 * "degree[s]", or a binary math operator.
 *
 * \return The value of the unit in radians.
 */
static double
unit_to_radians(const unit_t *u) {
    if (strcmp(u->type, "+") == 0) {
        double x = unit_to_radians(u->arg1);
        double y = unit_to_radians(u->arg2);
        return x + y;
    } else if (strcmp(u->type, "+") == 0) {
        double x = unit_to_radians(u->arg1);
        double y = unit_to_radians(u->arg2);
        return x - y;
    } else if (strcmp(u->type, "*") == 0) {
        double x = unit_to_radians(u->arg1);
        return x * u->value;
    } else if (strcmp(u->type, "/") == 0) {
        double x = unit_to_radians(u->arg1);
        return x / u->value;
    } else if (strncmp(u->type, "radian", 6) == 0) {
        return u->value;
    } else if (strncmp(u->type, "degree", 6) == 0) {
        return u->value * 2 * Pi / 360.0;
    } else {
        fprintf(stderr, "Warning: can't convert unit '%s' to radians\n", u->type);
        return 0.0;
    }
}

/**
 * NOTE: this function assumes device coordinates are pixels.
 */
static double
unit_to_npc_helper(double device_per_npc, double line_height_npc, unit_t *u) {
    if (strcmp(u->type, "+") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_height_npc, u->arg1);
        double y = unit_to_npc_helper(device_per_npc, line_height_npc, u->arg2);
        return x + y;
    } else if (strcmp(u->type, "+") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_height_npc, u->arg1);
        double y = unit_to_npc_helper(device_per_npc, line_height_npc, u->arg2);
        return x - y;
    } else if (strcmp(u->type, "*") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_height_npc, u->arg1);
        return x * u->value;
    } else if (strcmp(u->type, "/") == 0) {
        double x = unit_to_npc_helper(device_per_npc, line_height_npc, u->arg1);
        return x / u->value;
    } else if (strcmp(u->type, "npc") == 0) {
        return u->value;
    } else if (strcmp(u->type, "px") == 0) {
        return u->value / device_per_npc;
    } else if (strncmp(u->type, "line", 4) == 0) {
        return u->value * line_height_npc;
    } else {
        fprintf(stderr, "Warning: can't convert unit '%s' to radians\n", u->type);
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
    double line_height_npc = font_extents->height;

    double result = 0.0;

    if (dim == 'x') {
        result = unit_to_npc_helper(device_x_per_npc, line_height_npc, u);
    } else if (dim == 'y') {
        result = unit_to_npc_helper(device_y_per_npc, line_height_npc, u);
    } else {
        fprintf(stderr, "Warning: unknown dimension '%c'\n", dim);
    }

    free(font_extents);
    return result;
}

//
// graphics parameters
//

/**
 * Allocate a new parameter struct and set default values.
 *
 * \returns A pointer to the newly allocated \ref grid_par_t.
 */
grid_par_t*
new_grid_par(void) {
    grid_par_t *par = malloc(sizeof(grid_par_t));

    par->red = par->green = par->blue = 0.0;
    par->alpha = 1.0;

    memset(par->lty, '\0', GridShortNameLength);
    strncpy(par->lty, "solid", GridShortNameLength);

    return par;
}

/**
 * Safely set the value of a string parameter from a null-terminated string.
 * All parameter `char *` fields have length \ref GridShortNameLength.  This
 * function ensures that you don't write past the end of the field. If `source`
 * is too long, it's simply cut off, and `dest` retains a terminating null
 * character.
 *
 * \param dest Destination string.
 * \param source Source string.
 */
void
grid_par_set_str(char *dest, const char *source) {
    strncpy(dest, source, GridShortNameLength - 1);
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
 * \param angle The angle of rotation of the new viewpor.
 * \return A pointer to the newly allocated \ref grid_viewport_t.
 */
grid_viewport_t*
new_grid_viewport(unit_t *x, unit_t *y, unit_t *width, unit_t *height, unit_t *angle) {
    grid_viewport_t *vp = malloc(sizeof(grid_viewport_t));
    vp->x = x;
    vp->y = y;
    vp->width = width;
    vp->height = height;
    vp->angle = angle;
    vp->par = new_grid_par();

    memset(vp->name, '\0', GridLongNameLength);

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
                             unit(1, "npc"), unit(1, "npc"), unit(0, "radians"));
}

/**
 * Allocate a new \ref grid_viewport_t with the given name. 
 *
 * \return A pointer to the newly allocated \ref grid_viewport_t.
 * \see new_grid_viewport
 */
grid_viewport_t*
new_grid_named_viewport(const char *name, unit_t *x, unit_t *y, 
                        unit_t *width, unit_t *height, unit_t *angle) 
{
    grid_viewport_t *vp = new_grid_viewport(x, y, width, height, angle);
    strncpy(vp->name, name, GridLongNameLength - 1);
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
    strncpy(vp->name, name, GridLongNameLength - 1);
    return vp;
}

/**
 * Deallocate a \ref grid_viewport_t.
 *
 * \param vp A viewport.
 * \param free_units If `true`, then free units referenced by the viewport.
 */
void
free_grid_viewport(grid_viewport_t *vp, bool free_units) {
    if (free_units) {
        if (vp->x)
            free_unit(vp->x, true);
        if (vp->y)
            free_unit(vp->y, true);
        if (vp->width)
            free_unit(vp->width, true);
        if (vp->height)
            free_unit(vp->height, true);
        if (vp->angle)
            free_unit(vp->angle, true);
    }

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

    cairo_translate(gr->cr, unit_to_npc(gr->cr, 'x', vp->x),
                            unit_to_npc(gr->cr, 'y', vp->y));
    cairo_scale(gr->cr, unit_to_npc(gr->cr, 'x', vp->width),
                        unit_to_npc(gr->cr, 'y', vp->height));
    cairo_rotate(gr->cr, unit_to_radians(vp->angle));
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
    if (strncmp(this->vp->name, name, GridLongNameLength) == 0) {
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

    if (root->vp)
        free_grid_viewport(root->vp, true);

    free(root);
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
grid_line(grid_context_t* gr, unit_t *x1, unit_t *x2, unit_t *y1, unit_t *y2,
          grid_par_t *par)
{
    if (par) {
        cairo_set_source_rgba(gr->cr, par->red, par->green, par->blue, par->alpha);
        cairo_set_line_width(gr->cr, unit_to_npc(gr->cr, 'x', unit(2, "px")));
    } else {
        // TODO fall back on viewport parameters
    }

    cairo_new_path(gr->cr);
    cairo_move_to(gr->cr, unit_to_npc(gr->cr, 'x', x1),
                          unit_to_npc(gr->cr, 'y', y1));
    cairo_line_to(gr->cr, unit_to_npc(gr->cr, 'x', x2),
                          unit_to_npc(gr->cr, 'y', y2));

    printf("line width: %f\n", cairo_get_line_width(gr->cr));

    cairo_stroke(gr->cr);
}
