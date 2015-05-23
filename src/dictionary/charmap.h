#include <stdlib.h>

struct char_map;

struct char_map * char_map_init();

int char_map_capacity();

int char_map_count(struct char_map *map);

void char_map_done(struct char_map *map);

int char_map_put(struct char_map *map, wchar_t key, char value);

int char_map_get(struct char_map *map, wchar_t key, char *value);

