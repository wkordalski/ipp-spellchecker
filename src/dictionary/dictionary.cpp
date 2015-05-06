#include "dictionary.h"


struct dictionary
{
    wchar_t *word;
};

int dictionary_insert(struct dictionary *dict, const wchar_t *word)
{
    dict->word = wcsdup(word);
}
