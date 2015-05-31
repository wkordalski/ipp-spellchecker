#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "dictionary.h"
#include "word_list.h"
#include "trie.h"

#ifdef free
#undef free
#endif /* free */
#define free(ptr) _test_free(ptr, __FILE__, __LINE__)
void _test_free(void* const ptr, const char* file, const int line);

/**
  Struktura przechowująca słownik.
  
  Słowa są przechowywane w drzewie TRIE.
 */
struct dictionary
{
    struct trie_node *root;      ///< Korzeń drzewa TRIE
};

static void dictionary_new_done_test(void **state)
{
    struct dictionary *dict = dictionary_new();
    assert_true(dict->root != NULL);
    dictionary_done(dict);
}

static void dictionary_insert_test(void **state)
{
    struct dictionary *dict = dictionary_new();
    dictionary_insert(dict, L"słowo");
    assert_true(trie_find(dict->root, L"słowo"));
    dictionary_done(dict);
}

static void dictionary_delete_test(void **state)
{
    struct dictionary *dict = dictionary_new();
    dictionary_insert(dict, L"słowo");
    dictionary_delete(dict, L"słowo");
    assert_false(trie_find(dict->root, L"słowo"));
    dictionary_done(dict);
}

static void dictionary_find_test(void **state)
{
    struct dictionary *dict = dictionary_new();
    dictionary_insert(dict, L"słowo");
    assert_false(dictionary_find(dict, L"bazgroł"));
    assert_true(dictionary_find(dict, L"słowo"));
    dictionary_done(dict);
}

static void dictionary_hints_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF-8");
    struct dictionary *dict = dictionary_new();
    dictionary_insert(dict, L"ala");
    dictionary_insert(dict, L"ąla");
    dictionary_insert(dict, L"bla");
    dictionary_insert(dict, L"ble");
    struct word_list list;
    dictionary_hints(dict, L"bla", &list);
    assert_int_equal(word_list_size(&list), 4);
    assert_true(wcscmp(word_list_get(&list)[0], L"ala")==0);
    assert_true(wcscmp(word_list_get(&list)[1], L"ąla")==0);
    assert_true(wcscmp(word_list_get(&list)[2], L"bla")==0);
    assert_true(wcscmp(word_list_get(&list)[3], L"ble")==0);
    word_list_done(&list);
    dictionary_done(dict);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(dictionary_new_done_test),
        cmocka_unit_test(dictionary_insert_test),
        cmocka_unit_test(dictionary_delete_test),
        cmocka_unit_test(dictionary_find_test),
        cmocka_unit_test(dictionary_hints_test),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
