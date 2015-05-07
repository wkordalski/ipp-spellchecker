/** @file
  Prosta implementacja słownika.
  Słownik składa się tylko z jednego słowa.

  @ingroup dictionary
  @author Jakub Pawlewicz <pan@mimuw.edu.pl>
  @copyright Uniwerstet Warszawski
  @date 2015-05-06
 */

#include "dictionary.h"
#include <stdlib.h>

#define _GNU_SOURCE

/**
  Struktura przechowująca słownik.
  */
struct dictionary
{
    wchar_t *word;          /**< Jedyne słowo w słowniku */
};

/** @name Funkcje pomocnicze
  @{
  */
/**
  Czyszczenie pamięci słownika
  @param[in,out] dict słownik
  */
static void dictionary_free(struct dictionary *dict)
{
    if (dict->word)
        free(dict->word);
}
/**@}*/
/** @name Elementy interfejsu */
struct dictionary * dictionary_new()
{
    struct dictionary *dict =
        (struct dictionary *) malloc(sizeof(struct dictionary));
    dict->word = NULL;
    return dict;
}

void dictionary_done(struct dictionary *dict)
{
    dictionary_free(dict);
    free(dict);
}

void dictionary_insert(struct dictionary *dict, const wchar_t *word)
{
    dictionary_free(dict);
    dict->word = wcsdup(word);
}

const wchar_t* dictionary_getword(struct dictionary *dict)
{
    return dict->word;
}
/**@}*/
