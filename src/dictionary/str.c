/** @file
    Implementacja dynamicznego string'a.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-15
 */

#include "str.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

struct string
{
    wchar_t *buffer;
    size_t size;
    size_t capacity;
};

struct string *string_make(const wchar_t *s)
{
    struct string *r = malloc(sizeof(struct string));
    r->capacity = 16;
    int sl = wcslen(s);
    if(sl > 16) r->capacity = sl+1;
    r->size = sl;
    r->buffer = malloc(r->capacity * sizeof(wchar_t));
    memcpy(r->buffer, s, sizeof(wchar_t)*sl);
    r->buffer[sl] = 0;
    return r;
}

void string_done(struct string *s)
{
    free(s->buffer);
    free(s);
}

wchar_t * string_undress(struct string *s)
{
    wchar_t *t = s->buffer;
    free(s);
    return t;
}

int string_reserve(struct string *s, size_t c)
{
    if(s->size+1 > c) c = s->size+1;
    void **na = malloc(c * sizeof(wchar_t));
    memcpy(na, s->buffer, s->size * sizeof(wchar_t));
    free(s->buffer);
    na[s->size] = 0;
    s->buffer = na;
    s->capacity = c;
    return c;
}

void string_append(struct string *s, wchar_t c)
{
    if(s->size+1 >= s->capacity)
        string_reserve(s, s->capacity * 2);
    s->buffer[s->size++] = c;
    s->buffer[s->size] = 0;
}

void string_clear(struct string *s)
{
    s->size = 0;
    s->buffer[0] = 0;
}

int string_serialize(struct string *s, FILE *file)
{
    wchar_t *ss = s->buffer;
    while(*ss != 0)
    {
        if(fputwc(*ss, file) < 0) return -1;
        ss++;
    }
    assert(ss-s->buffer == s->size);
    if(fputwc(0, file) < 0) return -1;
    return 0;
}

struct string * string_deserialize(FILE *file)
{
    struct string *s = string_make(L"");
    while(1)
    {
        int c = fgetwc(file);
        if(c < 0) goto fail;
        if(c == 0) break;
        string_append(s, c);
    }
    return s;
fail:
    string_done(s);
    return NULL;
}