#include "charmap.h"

#include <assert.h>
#include <stdlib.h>

#define CHAR_MAP_SIZE 256

struct char_map
{
    int count;
    int counts[CHAR_MAP_SIZE];
    wchar_t keys[CHAR_MAP_SIZE][CHAR_MAP_SIZE];
    char values[CHAR_MAP_SIZE][CHAR_MAP_SIZE];
};

int char_map_capacity()
{
    return CHAR_MAP_SIZE;
}

int char_map_count(struct char_map* map)
{
    return map->count;
}

struct char_map * char_map_init()
{
    struct char_map *map = malloc(sizeof(struct char_map));
    map->count = 0;
    for(int i = 0; i < CHAR_MAP_SIZE;  i++)
    {
        map->counts[i] = 0;
    }
    return map;
}

void char_map_done(struct char_map *map)
{
    free(map);
}

int char_map_get_position_in_bucket(wchar_t *bucket, wchar_t key, int begin, int end)
{
    if(end == begin) return begin;
    if(end - begin <= 2)
    {
        if(begin < end && bucket[begin] >= key) return begin;
        else begin++;
        if(begin < end && bucket[begin] >= key) return begin;
        else begin++;
        return end;
    }
    int middle = (begin+end)/2;
    if(bucket[middle] == key) return middle;
    if(bucket[middle] > key) return char_map_get_position_in_bucket(bucket, key, begin, middle);
    else return char_map_get_position_in_bucket(bucket, key, middle + 1, end);
}

int char_map_put(struct char_map *map, wchar_t key, char value)
{
    int bucket = key % CHAR_MAP_SIZE;
    int place = char_map_get_position_in_bucket(map->keys[bucket], key, 0, map->counts[bucket]);
    if(map->counts[bucket] > place && map->keys[bucket][place] == key) return 0;
    assert(map->counts[bucket] < CHAR_MAP_SIZE - 1);
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
    int bucket = key % CHAR_MAP_SIZE;
    int place = char_map_get_position_in_bucket(map->keys[bucket], key, 0, map->counts[bucket]);
    if(map->keys[bucket][place] == key)
    {
        *value = map->values[bucket][place];
        return 1;
    }
    return 0;
}
