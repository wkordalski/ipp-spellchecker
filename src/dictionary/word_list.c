/** @file
  Implementacja listy słów.

  @ingroup dictionary
  @author Jakub Pawlewicz <pan@mimuw.edu.pl>
          Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-05-10
 */

#include "word_list.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../testable.h"


/** @name Elementy interfejsu 
   @{
 */

void word_list_init(struct word_list *list)
{
    list->size = 0;
    list->capacity = 16;
    list->array = malloc(sizeof(wchar_t*)*(list->capacity));
}

void word_list_done(struct word_list *list)
{
    for(int i = 0; i < list->size; i++)
    {
        free((void*)(list->array[i]));
    }
    free(list->array);
}

int word_list_add(struct word_list *list, const wchar_t *word)
{
    if(list->size >= list->capacity)
    {
        // Enlarge the list
        list->capacity *= 2;
        const wchar_t ** na = malloc(sizeof(wchar_t*)*(list->capacity));
        if(na == NULL) return 0;
        for(int i = 0; i < list->size; i++)
            na[i] = list->array[i];
        free(list->array);
        list->array = na;
    }
    int len = wcslen(word) + 1;
    wchar_t *copy = malloc(sizeof(wchar_t)*len);
    memcpy(copy, word, sizeof(wchar_t)*len);
    list->array[list->size++] = copy;
    return 1;
}

size_t word_list_size(const struct word_list *list)
{
    return list->size;
}

const wchar_t * const * word_list_get(const struct word_list *list)
{
    return list->array;
}

/**@}*/
