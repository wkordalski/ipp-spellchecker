/** @file
  Implementacja char-mapy.

  @ingroup dictionary
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-05-24
 */


#include "charmap.h"

#include <assert.h>
#include <stdlib.h>

#include "../testable.h"

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

/** @name Funkcje pomocnicze
 * @{
 */

/**
 * Zwraca miejsce w koszyku, na którym powinien się znaleźć podany element.
 * 
 * @param[in] bucket Koszyk, w którym szukać.
 * @param[in] key Element, którego szukać.
 * @param[in] begin Początek przedziału, w którym może być dany element.
 * @param[in] end Koniec przedziału, w którym może być dany element.
 * 
 * @return Indeks, gdzie powinien się znaleźć (lub jest) dany element.
 */
static int char_map_get_position_in_bucket(const wchar_t const *bucket, wchar_t key, int begin, int end)
{
    assert(bucket != NULL);
    assert(begin <= end);
    if(end == begin) return begin;
    if(end - begin <= 2)
    {
        for(int i = begin; i < end; i++)
        {
            if(bucket[i] >= key) return i;
        }
        return end;
    }
    int middle = (begin+end)/2;
    if(bucket[middle] == key) return middle;
    if(bucket[middle] > key) return char_map_get_position_in_bucket(bucket, key, begin, middle);
    else return char_map_get_position_in_bucket(bucket, key, middle + 1, end);
}

/**
 * @}
 */

/** @name Elementy interfejsu 
   @{
 */

struct char_map * char_map_init()
{
    struct char_map *map = malloc(sizeof(struct char_map));
    map->count = 0;
    for(int i = 0; i < CHAR_MAP_CAPACITY;  i++)
    {
        map->counts[i] = 0;
    }
    return map;
}

void char_map_done(struct char_map *map)
{
    assert(map != NULL);
    free(map);
}

int char_map_capacity()
{
    return CHAR_MAP_CAPACITY;
}

int char_map_size(struct char_map* map)
{
    assert(map != NULL);
    return map->count;
}

int char_map_put(struct char_map *map, wchar_t key, char value)
{
    assert(map != NULL);
    int bucket = key % CHAR_MAP_CAPACITY;
    int place = char_map_get_position_in_bucket(map->keys[bucket], key, 0, map->counts[bucket]);
    if(map->counts[bucket] > place && map->keys[bucket][place] == key) return 0;
    assert(map->counts[bucket] < CHAR_MAP_CAPACITY - 1);
    for(int i = map->counts[bucket] - 1; i >= place; i--)
    {
        map->keys[bucket][i + 1] = map->keys[bucket][i];
    }
    map->keys[bucket][place] = key;
    for(int i = map->counts[bucket] - 1; i >= place; i--)
    {
        map->values[bucket][i + 1] = map->values[bucket][i];
    }
    map->values[bucket][place] = value;
    map->counts[bucket]++;
    map->count++;
    return 1;
}

int char_map_get(struct char_map *map, wchar_t key, char *value)
{
    assert(map != NULL);
    int bucket = key % CHAR_MAP_CAPACITY;
    int place = char_map_get_position_in_bucket(map->keys[bucket], key, 0, map->counts[bucket]);
    if(place < map->counts[bucket] && map->keys[bucket][place] == key)
    {
        *value = map->values[bucket][place];
        return 1;
    }
    return 0;
}

/**
 * @}
 */

