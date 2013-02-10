#include "griddle.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define Pi 3.141593

//
// units
//

// NOTE all unit functions allocate memory

unit_t*
unit(double value, const char *type) {
    unit_t *u = malloc(sizeof(unit_t));
    u->value = value;
    strncpy(u->type, type, GridShortNameLength - 1);
    u->type[GridShortNameLength - 1] = '\0';

    return u;
}

unit_t*
unit_add(unit_t *arg1, unit_t *arg2) {
    unit_t *u = unit(0.0, "+");
    u->arg1 = arg1;
    u->arg2 = arg2;

    return u;
}

unit_t*
unit_sub(unit_t *arg1, unit_t *arg2) {
    unit_t *u = unit(0.0, "-");
    u->arg1 = arg1;
    u->arg2 = arg2;

    return u;
}

unit_t*
unit_mul(const unit_t* u, double x) {
    return unit(u->value * x, u->type);
}

unit_t*
unit_div(const unit_t* u, double x) {
    return unit(u->value / x, u->type);
}

static double
unit_to_radians(const unit_t *u) {
    if (strcmp(u->type, "radians") == 0) {
        return u->value;
    } else if (strcmp(u->type, "degrees") == 0) {
        return u->value * 2 * Pi / 360.0;
    } else {
        fprintf(stderr, "Warning: can't convert unit '%s' to radians\n", u->type);
        return 0.0;
    }
}

//
// graphics parameters
//

// allocate a new parameter struct and set default values.
//
grid_par_t*
new_grid_par(void) {
    grid_par_t *par = malloc(sizeof(grid_par_t));
    strncpy(par->lty, "solid", GridShortNameLength);

    return par;
}

// safely set the value of a string parameter from a null-terminated string
//
void
grid_par_set_str(char *dest, const char *source) {
    strncpy(dest, source, GridShortNameLength - 1);
}

//
// viewports
//

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

grid_viewport_t*
new_grid_default_viewport() {
    return new_grid_viewport(unit(0, "npc"), unit(0, "npc"), 
                             unit(1, "npc"), unit(1, "npc"), unit(0, "radians"));
}

grid_viewport_t*
new_grid_named_viewport(const char *name, unit_t *x, unit_t *y, 
                        unit_t *width, unit_t *height, unit_t *angle) 
{
    grid_viewport_t *vp = new_grid_viewport(x, y, width, height, angle);
    strncpy(vp->name, name, GridLongNameLength - 1);
    return vp;
}

grid_viewport_t*
new_grid_named_default_viewport(const char *name) {
    grid_viewport_t *vp = new_grid_default_viewport();
    strncpy(vp->name, name, GridLongNameLength - 1);
    return vp;
}

static grid_viewport_node_t*
new_grid_viewport_node(grid_viewport_t *vp) {
    grid_viewport_node_t *node = malloc(sizeof(grid_viewport_node_t));
    node->vp = vp;
    node->parent = node->sibling = node->child = NULL;

    return node;
}

static void
grid_apply_viewport_transform(grid_context_t *gr, grid_viewport_t *vp) {
    cairo_save(gr->cr);
    cairo_rotate(gr->cr, unit_to_radians(vp->angle));
}

void
grid_push_viewport(grid_context_t *gr, grid_viewport_t *vp) {
    grid_viewport_node_t *node = new_grid_viewport_node(vp);

    if (gr->current_node->child) {
        node->sibling = gr->current_node->child;
    }

    gr->current_node->child = node;
    node->parent = gr->current_node;
    gr->current_node = node;

    grid_apply_viewport_transform(gr, vp);
}

grid_viewport_t*
grid_pop_viewport_1(grid_context_t *gr) {
    grid_viewport_node_t *node;
    grid_viewport_t *vp = NULL;

    if (gr->current_node == gr->root_node) {
        fprintf(stderr, "Warning: attempted to pop root viewport from the stack in %s:%d\n",
                __FILE__, __LINE__);
    } else {
        node = gr->current_node;
        vp = node->vp;
        gr->current_node = gr->current_node->parent;
        free(node);
        cairo_restore(gr->cr);
    }

    return vp;
}

grid_viewport_t*
grid_pop_viewport(grid_context_t *gr, int n) {
    grid_viewport_t *vp, *temp_vp = NULL;

    int i;
    for (i = 0; i < n; i++) {
        if ((temp_vp = grid_pop_viewport_1(gr)) != NULL)
            vp = temp_vp;
        else
            // grid_pop_viewport_1 will print an error message
            break;
    }

    return vp;
}

bool
grid_up_viewport_1(grid_context_t *gr) {
    if (gr->current_node == gr->root_node) {
        fprintf(stderr, "Warning: attempted to move up from root viewport in %s:%d\n",
                __FILE__, __LINE__);
        return false;
    }

    gr->current_node = gr->current_node->parent;
    cairo_restore(gr->cr);
    return true;
}

// returns the number of viewports traversed
//
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

// a type for creating a linked list of viewports, for use in grid_viewport_bfs
//

typedef struct {
    grid_viewport_node_t *first, *last;
} grid_viewport_list_t;

static grid_viewport_list_t*
new_grid_viewport_list(grid_viewport_t *vp) {
    grid_viewport_node_t *node = new_grid_viewport_node(vp);
    grid_viewport_list_t *list = malloc(sizeof(grid_viewport_list_t));
    list->first = node;
    list->last = node;

    return list;
}

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

static void
grid_viewport_list_append(grid_viewport_list_t *list, grid_viewport_t *vp) {
    grid_viewport_node_t *node = new_grid_viewport_node(vp);
    list->last->child = node;
    list->last = node;
}

static void
grid_viewport_list_concat(grid_viewport_list_t *list1, grid_viewport_list_t *list2) {
    list1->last->child = list2->first;
   list1->last = list2->last;
}

// perform a depth first search for a viewport with the given name.
// *level is incremented each time the algorithm recurses on a child viewport
// and is set to -1 if no matching viewport is found. path is a list of nodes
// traversed to find the target. If the return value is NULL, then the values
// of plevel and path are undefined.
//
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
            this_child = this_child->sibling;
        }
    }

    return NULL;
}

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
    }

    free_grid_viewport_list(path);
    return n;
}

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
    }

    free_grid_viewport_list(path);
    return level;
}

//
// draw functions
//

grid_context_t*
new_grid_context(int width_px, int height_px) {
    grid_context_t *gr = malloc(sizeof(grid_context_t));
    gr->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width_px, height_px);
    gr->cr = cairo_create(gr->surface);

    // put the user scale in npc
    cairo_scale(gr->cr, width_px, height_px);

    grid_viewport_t *root = new_grid_default_viewport();
    strncpy(root->name, "root", GridLongNameLength - 1);
    gr->root_node = new_grid_viewport_node(root);
    gr->current_node = gr->root_node;

    return gr;
}
