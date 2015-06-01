/** @file
  Test implementacji charmap'y.
  
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
#include "charmap.h"

/** Pojemność char-mapy */
#define CHAR_MAP_CAPACITY 256

/**
 * Reprezentuje char-mapę.
 */
struct char_map
{
    int count;                                          ///< Ilość elementów w mapie
    int counts[CHAR_MAP_CAPACITY];                      ///< Ilość elementów w koszykach
    wchar_t keys[CHAR_MAP_CAPACITY][CHAR_MAP_CAPACITY]; ///< Elementy w mapie
    char values[CHAR_MAP_CAPACITY][CHAR_MAP_CAPACITY];  ///< Wartości przypisane elementom mapy.
};

extern int char_map_get_position_in_bucket(const wchar_t const *bucket, wchar_t key, int begin, int end);
extern int char_map_bucket_size(struct char_map *map, int bucket);

const wchar_t sample_bucket[] = { L'a', L'n', L'o', L'r' };

/**
 * Testuje funkcję znajdującą wpis odpowiadający danej literze w koszyku.
 */
static void char_map_get_position_in_bucket_tests(void **state)
{
    // begin == end
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'x', 2, 2), 2);
    // end - begin <= 2
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'o', 1, 3), 2);
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'a', 1, 2), 1);
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'r', 1, 2), 2);
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'g', 0, 2), 1);
    // recursive
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'b', 0, 4), 1);
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'w', 0, 4), 4);
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'n', 0, 4), 1);
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'a', 0, 4), 0);
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'o', 0, 4), 2);
    assert_int_equal(char_map_get_position_in_bucket(sample_bucket, L'r', 0, 4), 3);
}
/**
 * Testuje tworzenie nowej char-mapy.
 */
static void char_map_init_tests(void **state)
{
    struct char_map *map = char_map_init();
    assert_true(map != NULL);
    assert_int_equal(char_map_size(map), 0);
    for(int i = 0; i < char_map_capacity(); i++)
    {
        assert_int_equal(map->counts[i], 0);
    }
    char_map_done(map);
}
/**
 * Testuje wstawianie elementu do mapy.
 */
static void char_map_put_tests(void **state)
{
    struct char_map *map = char_map_init();
    assert_true(map != NULL);
    assert_true(char_map_put(map, L'a'+char_map_capacity(), 'a'));
    assert_false(char_map_put(map, L'a'+char_map_capacity(), 'a'));
    assert_int_equal(map->counts[L'a'%char_map_capacity()], 1);
    assert_true(char_map_put(map, L'a'+2*char_map_capacity(), 'b'));
    assert_int_equal(map->counts[L'a'%char_map_capacity()], 2);
    assert_true(char_map_put(map, L'a', 'c'));
    assert_int_equal(map->counts[L'a'%char_map_capacity()], 3);
    for(int i = 0; i < char_map_capacity(); i++)
    {
        if(i != L'a'%char_map_capacity())
            assert_int_equal(map->counts[i], 0);
    }
    char_map_done(map);
}

/**
 * Tworzy przykładowe środowisko.
 */
static int char_map_setup(void ** state)
{
    struct char_map *map = char_map_init();
    assert_true(map != NULL);
    assert_true(char_map_put(map, L'a', 'a'));
    assert_true(char_map_put(map, L'r', 'r'));
    assert_true(char_map_put(map, L'o', 'o'));
    assert_true(char_map_put(map, L'n', 'n'));
    *state = map;
    return 0;
}
/**
 * Usuwa przykładowe środowisko.
 */
static int char_map_teardown(void **state)
{
    struct char_map *map = *state;
    char_map_done(map);
    return 0;
}

/**
 * Testuje znajdowanie powiązania w mapie.
 */
static void char_map_get_test(void **state)
{
    struct char_map *map = *state;
    char val;
    assert_true(char_map_get(map, L'a', &val));
    assert_true(val == 'a');
    assert_false(char_map_get(map, L'b', &val));
    assert_false(char_map_get(map, L'a'+char_map_capacity(), &val));
}

/**
 * Testuje funkcję zwracającą rozmiar mapy.
 */
static void char_map_size_test(void **state)
{
    struct char_map *map = *state;
    assert_int_equal(char_map_size(map), 4);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(char_map_get_position_in_bucket_tests),
        cmocka_unit_test(char_map_init_tests),
        cmocka_unit_test(char_map_put_tests),
        cmocka_unit_test_setup_teardown(char_map_get_test, char_map_setup, char_map_teardown),
        cmocka_unit_test_setup_teardown(char_map_size_test, char_map_setup, char_map_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
