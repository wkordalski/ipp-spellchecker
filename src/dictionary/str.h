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

struct string;

struct string *string_make(const wchar_t *s);

void string_done(struct string *s);

wchar_t * string_undress(struct string *s);

int string_reserve(struct string *s, size_t c);

void string_append(struct string *s, wchar_t c);

void string_clear(struct string *s);

int string_serialize(struct string *s, FILE *file);

struct string * string_deserialize(FILE *file);

#endif /* DICTIONARY_STRING_H */