#include "grid_units.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Allocate a new \ref unit_t with the given type. Currently supported unit types
 * are "px" and "line".
 */
unit_t*
unit(double value, const char *type) {
    unit_t *u = malloc(sizeof(unit_t));
    u->value = value;
    u->type = malloc(strlen(type) + 1);
    strcpy(u->type, type);

    u->arg1 = NULL;
    u->arg2 = NULL;

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
 * Deallocate a \ref unit_t. Don't try to free `*type` since the most common
 * use is to construct a unit using a string literal.
 */
void
free_unit(unit_t *u) {
    if (u->arg1)
        free_unit(u->arg1);
    if (u->arg2)
        free_unit(u->arg2);

    free(u);
}

/**
 * Recursively find the size of a unit array.
 */
int
unit_array_size(const unit_array_t *u) {
    if (u->size > 0)
        return u->size;
    else if (u->arg1)
        return unit_array_size(u->arg1);
    else
        return 0;
}

/**
 * Allocate a new \ref unit_array_t with the given type.
 */
unit_array_t*
unit_array(int size, const double *values, const char *type) {
    unit_array_t *u = malloc(sizeof(unit_array_t));
    u->size = size;

    double *my_values = malloc(size * sizeof(double));
    int i;
    for (i = 0; i < size; i++) {
        my_values[i] = values[i];
    }
    u->values = my_values;

    u->type = malloc(strlen(type) + 1);
    strcpy(u->type, type);

    u->arg1 = NULL;
    u->arg2 = NULL;

    return u;
}

/**
 * Allocate a new \ref unit_array_t representing the sum of its arguments.
 *
 * \return A unit representing arg1 + arg2. Returns `NULL` if arg1 and arg2
 * have different lengths.
 */
unit_array_t*
unit_array_add(unit_array_t *arg1, unit_array_t *arg2) {
    if (arg1->size != arg2->size) {
        fprintf(stderr, "Warning: can't add arrays of different lengths "
                        "(%d and %d).\n", arg1->size, arg2->size);
        return NULL;
    }

    unit_array_t *u = malloc(sizeof(unit_array_t));
    u->size = 0;
    u->values = NULL;
    u->type = "+";
    u->arg1 = arg1;
    u->arg2 = arg2;

    return u;
}

/**
 * Allocate a new \ref unit_array_t representing the difference of its arguments.
 *
 * \return A unit representing arg1 - arg2. Returns `NULL` if arg1 and arg2
 * have different lengths.
 */
unit_array_t*
unit_array_sub(unit_array_t *arg1, unit_array_t *arg2) {
    if (arg1->size != arg2->size) {
        fprintf(stderr, "Warning: can't difference arrays of different lengths "
                        "(%d and %d).\n", arg1->size, arg2->size);
        return NULL;
    }

    unit_array_t *u = malloc(sizeof(unit_array_t));
    u->size = 0;
    u->values = NULL;
    u->type = "-";
    u->arg1 = arg1;
    u->arg2 = arg2;

    return u;
}

/**
 * Allocate a new \ref unit_array_t representing the value of `u` multiplied by
 * a scalar.
 * 
 * \return A unit representing u * x.
 */
unit_array_t*
unit_array_mul(unit_array_t* u, double x) {
    double *value = malloc(sizeof(double));
    *value = x;

    unit_array_t *v = unit_array(1, value, "*");
    v->arg1 = u;

    return v;
}

/**
 * Allocate a new \ref unit_array_t representing the value of `u` divided by
 * a scalar.
 * 
 * \return A unit representing u / x.
 */
unit_array_t*
unit_array_div(unit_array_t* u, double x) {
    double *value = malloc(sizeof(double));
    *value = x;

    unit_array_t *v = unit_array(1, value, "/");
    v->arg1 = u;

    return v;
}

/**
 * Deallocate a \ref unit_t. Doesn't free underlying data.
 */
void
free_unit_array(unit_array_t *u) {
    if (u->arg1)
        free_unit_array(u->arg1);
    if (u->arg2)
        free_unit_array(u->arg2);

    free(u);
}
