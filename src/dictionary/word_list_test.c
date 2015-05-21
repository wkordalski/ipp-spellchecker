#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "word_list.h"

const wchar_t* test   = L"Test string";
const wchar_t* first  = L"First string";
const wchar_t* second = L"Second string";
const wchar_t* third  = L"Third string";

static void word_list_init_test(void** state) {
    struct word_list l;
    word_list_init(&l);
    assert_int_equal(word_list_size(&l), 0);
    word_list_done(&l);
}

static void word_list_add_test(void** state) {
    struct word_list l;
    word_list_init(&l);
    word_list_add(&l, test);
    assert_int_equal(word_list_size(&l), 1);
    assert_true(wcscmp(test, word_list_get(&l)[0]) == 0);
    word_list_done(&l);
}


static int word_list_setup(void **state) {
    struct word_list *l = malloc(sizeof(struct word_list));
    if (!l) 
        return -1;
    word_list_init(l);
    word_list_add(l, first);
    word_list_add(l, second);
    word_list_add(l, third);
    *state = l;
    return 0;
}
static int word_list_teardown(void **state) {
    struct word_list *l = *state;
    word_list_done(l);
    free(l);
    return 0;
}

static void word_list_get_test(void** state) {
    struct word_list *l = *state;
    assert_true(wcscmp(first, word_list_get(l)[0]) == 0);
    assert_true(wcscmp(second, word_list_get(l)[1]) == 0);
    assert_true(wcscmp(third, word_list_get(l)[2]) == 0);
}

static void word_list_repeat_test(void** state) {
    struct word_list *l = *state;
    word_list_add(l, third);
    assert_int_equal(word_list_size(l), 4);
    assert_true(wcscmp(third, word_list_get(l)[3]) == 0);
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(word_list_init_test),
        cmocka_unit_test(word_list_add_test),
        cmocka_unit_test_setup_teardown(word_list_get_test, word_list_setup, word_list_teardown),
        cmocka_unit_test_setup_teardown(word_list_repeat_test, word_list_setup, word_list_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
