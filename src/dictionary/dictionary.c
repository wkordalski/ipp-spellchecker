/** @file
  Prosta implementacja słownika.
  */

#include "dictionary.h"
#include <stdlib.h>

#define _GNU_SOURCE

struct dictionary
{
    wchar_t *word;
};

struct dictionary * dictionary_new()
{
    struct dictionary *dict =
        (struct dictionary *) malloc(sizeof(struct dictionary));
    dict->word = NULL;
}

static void dictionary_free(struct dictionary *dict)
{
    if (dict->word)
        free(dict->word);
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
