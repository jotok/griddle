#ifndef GridUnits_h
#define GridUnits_h

typedef struct __unit_t {
    double value;
    char *type;
    struct __unit_t *arg1, *arg2;
} unit_t;

typedef struct __unit_array_t {
    double *values;
    int size;
    char *type;
    struct __unit_array_t *arg1, *arg2;
} unit_array_t;

#define Unit(X,T) ((unit_t){.value = X, .type = T})

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
free_unit(unit_t*);

#define UnitArray(N,A,T) ((unit_array_t){.size = N, .values = A, .type = T})

int
unit_array_size(const unit_array_t*);

unit_array_t*
unit_array(int, const double*, const char*);

unit_array_t*
unit_array_add(unit_array_t*, unit_array_t*);

unit_array_t*
unit_array_sub(unit_array_t*, unit_array_t*);

unit_array_t*
unit_array_mul(unit_array_t*, double);

unit_array_t*
unit_array_div(unit_array_t*, double);

void
free_unit_array(unit_array_t*);

#endif
