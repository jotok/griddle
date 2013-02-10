#include "griddle.h"
#include "CuTest.h"

#include <stdio.h>
#include <stdlib.h>

void
test_units(CuTest *tc) {
    unit_t *u1 = unit(2.0, "px");
    unit_t *u2 = unit(3.0, "npc");

    CuAssertDblEquals(tc, u1->value, 2.0, 1e-8);
    CuAssertStrEquals(tc, u1->type, "px");
    CuAssertPtrEquals(tc, u1->arg1, NULL);
    CuAssertPtrEquals(tc, u1->arg2, NULL);

    unit_t *u3 = unit_mul(u1, 2);
    CuAssertDblEquals(tc, u3->value, 4, 1e-8);
    free(u3);

    u3 = unit_div(u1, 2);
    CuAssertDblEquals(tc, u3->value, 1, 1e-8);
    free(u3);

    u3 = unit_add(u1, u2);
    CuAssertStrEquals(tc, u3->type, "+");
    CuAssertPtrEquals(tc, u3->arg1, u1);
    CuAssertPtrEquals(tc, u3->arg2, u2);
    free(u3);

    u3 = unit_sub(u1, u2);
    CuAssertStrEquals(tc, u3->type, "-");
    CuAssertPtrEquals(tc, u3->arg1, u1);
    CuAssertPtrEquals(tc, u3->arg2, u2);
    free(u3);

    free(u1);
    free(u2);
}

void
test_par(CuTest *tc) {
    grid_par_t *par = new_grid_par();
    grid_par_set_str(par->lty, "banana");
    CuAssertStrEquals(tc, par->lty, "banana");

    free(par);
}

void
test_grid_context_constructor(CuTest *tc) {
    grid_context_t *gr = new_grid_context(100, 100);

    CuAssertPtrNotNull(tc, gr);
    CuAssertPtrNotNull(tc, gr->surface);
    CuAssertPtrNotNull(tc, gr->cr);
    CuAssertPtrNotNull(tc, gr->root_node);
    CuAssertPtrNotNull(tc, gr->current_node);
}

void
test_grid_viewport_tree(CuTest *tc) {
    grid_context_t *gr = new_grid_context(100, 100);

    grid_viewport_t *apple = new_grid_named_default_viewport("apple");
    grid_push_viewport(gr, apple);

    CuAssertTrue(tc, gr->root_node != gr->current_node);
    CuAssertPtrEquals(tc, gr->current_node->vp, apple);
    CuAssertPtrEquals(tc, gr->root_node->child, gr->current_node);
    CuAssertPtrEquals(tc, gr->current_node->parent, gr->root_node);

    grid_viewport_t *banana = new_grid_named_default_viewport("banana");
    grid_viewport_t *carrot = new_grid_named_default_viewport("carrot");
    grid_push_viewport(gr, banana);
    grid_up_viewport_1(gr);
    grid_push_viewport(gr, carrot);

    CuAssertPtrEquals(tc, gr->current_node->vp, carrot);
    CuAssertPtrEquals(tc, gr->current_node->sibling->vp, banana);
    CuAssertPtrEquals(tc, gr->current_node->parent->vp, apple);
}

CuSuite*
grid_test_suite(void) {
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_units);
    SUITE_ADD_TEST(suite, test_par);
    SUITE_ADD_TEST(suite, test_grid_context_constructor);
    SUITE_ADD_TEST(suite, test_grid_viewport_tree);

    return suite;
}

int
main(void) {
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();
    
    CuSuiteAddSuite(suite, grid_test_suite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}
