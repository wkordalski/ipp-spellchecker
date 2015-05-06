/** @file
  Prosta implementacja słownika.
  Słownik składa się tylko z jednego słowa.
  @author Jakub Pawlewicz <pan@mimuw.edu.pl>
  @copyright Uniwerstet Warszawski
  @date 2015-05-06
 */

#include "dictionary.h"

struct dictionary
{
    wchar_t *word;          /**< Jedyne słowo w słowniku */
};

void dictionary_init(struct dictionary *dict)
{
    dict->word = NULL;
}


/**
  Czyszczenie pamięci słownika
  @param[in,out] dict słownik
  */
static void dictionary_free(struct dictionary *dict)
{
    if (dict->word)
        free(dict->word);
}

void dictionary_done(struct dictionary *dict)
{
    dictionary_free(dict);
    dict->word = NULL;
}

void dictionary_insert(struct dictionary *dict, const wchar_t *word)
{
    dictionary_free(dict);
    dict->word = wcsdup(word);
}

const wchar_t* dictionary_getword(struct dictionary *)
{
    return dict->word;
}
