/** @file
    Implementacja dynamicznej listy.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-03
 */

#include "list.h"

#include <stdlib.h>
#include <string.h>

/**
 * Struktura przechowująca listę słów.
 */
struct list
{
    /// Liczba słów.
    size_t size;
    /// Pojemność tablicy
    size_t capacity;
    /// Tablica słów.
    void **array;
};

struct list * list_init()
{
    struct list * ptr = malloc(sizeof(struct list));
    if(ptr == NULL) return NULL;
    ptr->size = 0;
    ptr->capacity = 16;
    ptr->array = malloc(ptr->capacity * sizeof(void*));
    if(ptr->array == NULL)
    {
        free(ptr);
        return NULL;
    }
    return ptr;
}

void list_done(struct list *l)
{
    if(l == NULL) return;
    if(l->array != NULL) free(l->array);
    free(l);
}

size_t list_add(struct list *l, void *e)
{
    if(l->size >= l->capacity)
    {
        size_t dcap = l->capacity * 2;
        if(l->size > dcap) dcap = l->size * 2;
        list_reserve(l, dcap);
    }
    l->array[l->size] = e;
    l->size++;
    return l->size;
}

size_t list_add_list(struct list *l, struct list *m)
{
    if(l->size + m->size >= l->capacity)
    {
        size_t dcap = l->capacity;
        if(m->capacity > dcap) dcap = m->capacity;
        if(l->size + m->size >= dcap) dcap *= 2;
        if(l->size + m->size >= dcap) dcap = (l->size + m->size)*2;
        list_reserve(l, dcap);
    }
    memcpy(l->array + l->size, m->array, m->size * sizeof(void*));
    l->size += m->size;
    return l->size;
}

size_t list_pop(struct list* l)
{
    if(l->size == 0) return 0;
    l->size--;
    return l->size;
}

void* list_top(struct list* l)
{
    if(l->size == 0) return NULL;
    return l->array[l->size - 1];
}


size_t list_size(const struct list *l)
{
    return l->size;
}

size_t list_capacity(const struct list *l)
{
    return l->capacity;
}

size_t list_reserve(struct list *l, size_t s)
{
    if(l->size > s) s = l->size;
    void **na = malloc(s * sizeof(void*));
    memcpy(na, l->array, l->size);
    free(l->array);
    l->array = na;
    return s;
}

void ** list_get(struct list *l)
{
    return l->array;
}

void list_sort(struct list *l, int (*f)(void*,void*))
{
    qsort(l->array, l->size, sizeof(void*), f);
}

void list_sort_and_unify(struct list *l, int (*f)(void*,void*), int (*g)(void*,void*), struct list *dups)
{
    if(l->size <= 1) return;
    list_sort(l, f);
    void **na = malloc(l->capacity * sizeof(void*));
    size_t ns = 1;
    void **oa = l->array;
    *na = *oa;
    na++;
    oa++;
    for(int i = 1; i < l->size; i++)
    {
        if(g(oa-1, oa) != 0)
        {
            *na = *oa;
            na++;
            ns++;
        }
        else
        {
            if(dups != NULL)
                list_add(dups, *oa);
        }
        oa++;
    }
    free(l->array);
    l->array = na;
    l->size = ns;
}

void list_clear(struct list *l)
{
    l->size = 0;
}