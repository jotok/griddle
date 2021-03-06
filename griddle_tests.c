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
    CuAssertDblEquals(tc, u3->value, 2, 1e-8);
    CuAssertPtrEquals(tc, u3->arg1, u1);
    CuAssertPtrEquals(tc, u3->arg2, NULL);
    free(u3);

    u3 = unit_div(u1, 2);
    CuAssertDblEquals(tc, u3->value, 2, 1e-8);
    CuAssertPtrEquals(tc, u3->arg1, u1);
    CuAssertPtrEquals(tc, u3->arg2, NULL);
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

    grid_viewport_t *apple = new_grid_default_viewport();
    grid_push_named_viewport(gr, "apple", apple);

    CuAssertTrue(tc, gr->root_node != gr->current_node);
    CuAssertStrEquals(tc, gr->current_node->name, "apple");
    CuAssertPtrEquals(tc, gr->root_node->child, gr->current_node);
    CuAssertPtrEquals(tc, gr->current_node->parent, gr->root_node);

    grid_viewport_t *banana = new_grid_default_viewport();
    grid_viewport_t *carrot = new_grid_default_viewport();
    grid_push_named_viewport(gr, "banana", banana);
    grid_up_viewport_1(gr);
    grid_push_named_viewport(gr, "carrot", carrot);

    CuAssertStrEquals(tc, "carrot", gr->current_node->name);
    CuAssertStrEquals(tc, "banana", gr->current_node->gege->name);
    CuAssertStrEquals(tc, "carrot", gr->current_node->gege->didi->name);
    CuAssertStrEquals(tc, "apple", gr->current_node->parent->name);

    bool result = grid_pop_viewport_1(gr);
    CuAssertTrue(tc, result);
    CuAssertStrEquals(tc, "apple", gr->current_node->name);
    CuAssertStrEquals(tc, "banana", gr->current_node->child->name);
    CuAssertPtrEquals(tc, NULL, gr->current_node->child->didi);

    int n = grid_down_viewport(gr, "durian");
    CuAssertIntEquals(tc, -1, n);
    CuAssertStrEquals(tc, "apple", gr->current_node->name);

    n = grid_down_viewport(gr, "banana");
    CuAssertIntEquals(tc, 1, n);
    CuAssertStrEquals(tc, "banana", gr->current_node->name);

    grid_push_named_viewport(gr, "carrot", carrot);
    n = grid_down_viewport(gr, "banana");
    CuAssertIntEquals(tc, -1, n);
    n = grid_seek_viewport(gr, "banana");
    CuAssertIntEquals(tc, 2, n);
    CuAssertStrEquals(tc, "banana", gr->current_node->name);

    free_grid_context(gr);
}

CuSuite*
grid_test_suite(void) {
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_units);
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
