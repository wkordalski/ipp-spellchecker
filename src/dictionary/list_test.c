/** @file
  Test implementacji listy.
  
  @ingroup dictionary
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-05-31
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "list.h"

static int sorter(const void *a, const void *b)
{
    int *A = *(int **)a;
    int *B = *(int **)b;
    return *A - *B;
}

static int comparer(const void *a, const void *b)
{
    int *A = *(int **)a;
    int *B = *(int **)b;
    return *A/10 - *B/10;
}

static int * intptr(int v)
{
    int *p = malloc(sizeof(int));
    *p = v;
    return p;
}

static int list_add_list_test(void **rubbish)
{
    int *a = intptr(21);
    int *b = intptr(18);
    int *c = intptr(1);
    int *d = intptr(7);
    int *e = intptr(42);
    struct list *x = list_init();
    list_add(x, a);
    list_add(x, b);
    list_add(x, c);
    list_add(x, d);
    list_add(x, e);
    
    int *f = intptr(48);
    int *g = intptr(3);
    int *h = intptr(12);
    int *i = intptr(9);    
    int *j = intptr(11);
    struct list *y = list_init();
    list_add(y, f);
    list_add(y, g);
    list_add(y, h);
    list_add(y, i);
    list_add(y, j);
    
    list_add_list(x, y);
    assert_int_equal(list_size(x), 10);
    int **L = list_get(x);
    assert_true(L[0] == a);
    assert_true(L[1] == b);
    assert_true(L[2] == c);
    assert_true(L[3] == d);
    assert_true(L[4] == e);
    assert_true(L[5] == f);
    assert_true(L[6] == g);
    assert_true(L[7] == h);
    assert_true(L[8] == i);
    assert_true(L[9] == j);
    
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    free(g);
    free(h);
    free(i);
    free(j);
    list_done(x);
    list_done(y);
}

static void list_reserve_grow_test(void **rubbish)
{
    struct list *l = list_init();
    int nc = 4 * list_capacity(l) + 7;
    list_reserve(l, nc);
    assert_true(list_capacity(l) >= nc);
    list_done(l);
}

static void list_reserve_copy_test(void **rubbish)
{
    int *a = intptr(21);
    int *b = intptr(18);
    int *c = intptr(1);
    int *d = intptr(7);
    int *e = intptr(42);
    struct list *x = list_init();
    list_add(x, a);
    list_add(x, b);
    list_add(x, c);
    list_add(x, d);
    list_add(x, e);
    int nc = 4 * list_capacity(x) + 7;
    list_reserve(x, nc);
    assert_int_equal(list_size(x), 5);
    int **L = list_get(x);
    assert_true(L[0] == a);
    assert_true(L[1] == b);
    assert_true(L[2] == c);
    assert_true(L[3] == d);
    assert_true(L[4] == e);
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    list_done(x);
}


static void list_sort_and_unify_test(void **rubbish)
{
    int *a = intptr(21);
    int *b = intptr(18);
    int *c = intptr(1);
    int *d = intptr(7);
    int *e = intptr(42);
    int *f = intptr(48);
    int *g = intptr(3);
    int *h = intptr(12);
    int *i = intptr(9);    
    int *j = intptr(11);
    
    struct list *l = list_init();
    list_add(l, a);
    list_add(l, b);
    list_add(l, c);
    list_add(l, d);
    list_add(l, e);
    list_add(l, f);
    list_add(l, g);
    list_add(l, h);
    list_add(l, i);
    list_add(l, j);
    
    struct list *k = list_init();
    list_sort_and_unify(l, sorter, comparer, k);
    assert_int_equal(list_size(l), 4);
    int **L = list_get(l);
    assert_true(L[0] == c);
    assert_true(L[1] == j);
    assert_true(L[2] == a);
    assert_true(L[3] == e);
    assert_int_equal(list_size(k), 6);
    L = list_get(k);
    assert_true(L[0] == g);
    assert_true(L[1] == d);
    assert_true(L[2] == i);
    assert_true(L[3] == h);
    assert_true(L[4] == b);
    assert_true(L[5] == f);
    
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    free(g);
    free(h);
    free(i);
    free(j);
    list_done(l);
    list_done(k);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(list_add_list_test),
        cmocka_unit_test(list_reserve_grow_test),
        cmocka_unit_test(list_reserve_copy_test),
        cmocka_unit_test(list_sort_and_unify_test)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
