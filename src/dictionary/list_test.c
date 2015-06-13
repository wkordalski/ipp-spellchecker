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
    assert_true(*(L[0]) == 1);
    assert_true(*(L[1]) == 11);
    assert_true(*(L[2]) == 21);
    assert_true(*(L[3]) == 42);
    assert_int_equal(list_size(k), 6);
    L = list_get(k);
    assert_true(*(L[0]) == 3);
    assert_true(*(L[1]) == 7);
    assert_true(*(L[2]) == 9);
    assert_true(*(L[3]) == 12);
    assert_true(*(L[4]) == 18);
    assert_true(*(L[5]) == 48);
    
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
        cmocka_unit_test(list_sort_and_unify_test)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
