/** @file
    Interfejs dynamicznego string'a.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-16
 */

#ifndef DICTIONARY_STRING_H
#define DICTIONARY_STRING_H

#include <wchar.h>

/**
 * String.
 */
struct string;

/**
 * Tworzy nowego stringa o ustalonej zawartości.
 * 
 * @param[in] s Początkowa wartość stringa.
 * @return Nowy string.
 */
struct string *string_make(const wchar_t *s);

/**
 * Usuwa podanego stringa.
 * 
 * @param[in] s String do usunięcia.
 */
void string_done(struct string *s);

/**
 * Usuwa stringa i zwraca jego wewnętrzną reprezentację.
 * 
 * @param[in] s String do usunięcia.
 * @return Wewnętrzna reprezentacja stringa.
 */
wchar_t * string_undress(struct string *s);

/**
 * Rezerwuje pamięć w stringu.
 * 
 * @param[in] s String.
 * @param[in] c Ilość pamięci do zarezerwowania.
 * @return Nowa pojemność stringa.
 */
int string_reserve(struct string *s, size_t c);

/**
 * Dodaje znak do stringa.
 * 
 * @param[in] s String.
 * @param[in] c Znak.
 */
void string_append(struct string *s, wchar_t c);

/**
 * Czyści stringa.
 * 
 * @param[in] s String.
 */
void string_clear(struct string *s);

/**
 * Zapisuje stringa do pliku.
 * 
 * @param[in] s String.
 * @param[in] file Plik.
 * @return <0 jeśli napotkano błąd, 0 w p.p.
 */
int string_serialize(struct string *s, FILE *file);

/**
 * Wczytuje stringa z pliku.
 * 
 * @param[in] file Plik.
 * @return NULL jeśli napotkano błąd, wskaźnik na stringa w p.p.
 */
struct string * string_deserialize(FILE *file);

#endif /* DICTIONARY_STRING_H */