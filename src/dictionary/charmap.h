/** @file
    Interfejs mapy ze znaku w liczbę 8-bitową.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-05-24
 */

#ifndef __CHARMAP_H__
#define __CHARMAP_H__

#include <stdlib.h>

/**
 * Typ reprezentujący mapę ze znaku na liczbę 8-bitową.
 */
struct char_map;

/**
 * Tworzy nową mapę char-map.
 * 
 * @return Wskaźnik na mapę.
 */
struct char_map * char_map_init();

/**
 * Zwraca pojemność mapy.
 * 
 * @return Pojemność mapy.
 */
int char_map_capacity();

/**
 * Zwraca ilość elementów w mapie.
 * 
 * @param map Mapa, której ilość elementów zwrócić.
 * @return Ilość elementów w mapie.
 */
int char_map_size(struct char_map *map);

/**
 * Usuwa mapę.
 * 
 * @param[in,out] map Mapa do usunięcia.
 */
void char_map_done(struct char_map *map);

/**
 * Wkłada element do mapy.
 * 
 * @param[in,out] map Mapa do której włożyć element.
 * @param[in] key Element do włożenia.
 * @param[in] value Wartość do przypisania elementowi.
 * 
 * @return 0 jeśli element był już w mapie, 0 otherwise.
 */
int char_map_put(struct char_map *map, wchar_t key, char value);

/**
 * Znajduje wartość przypisaną elementowi.
 * 
 * @param[in] map Mapa, w której szukać elementu.
 * @param[in] key Element, który znaleźć.
 * @param[out] value Wartość przypisana elementowi.
 * 
 * @return 0 jeśli element nie istnieje w mapie, 1 otherwise.
 */
int char_map_get(struct char_map *map, wchar_t key, char *value);

#endif /* __CHARMAP_H__ */