/** @file
  Prosta implementacja słownika.
  Słownik przechowuje listę słów.

  @ingroup dictionary
  @author Jakub Pawlewicz <pan@mimuw.edu.pl>
  @copyright Uniwerstet Warszawski
  @date 2015-05-11
  @todo Napisać efektywną implementację.
 */

#include "dictionary.h"
#include <stdio.h>
#include <stdlib.h>

#define _GNU_SOURCE

/**
  Struktura przechowująca słownik.
  Na razie prosta implementacja z użyciem listy słów.
 */
struct dictionary
{
    struct word_list list; ///< Lista przechowująca słowa w słowniku.
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
    word_list_done(&dict->list);
}

static void skip_equal(const wchar_t **a, const wchar_t **b)
{
    while (**a == **b && **a != L'\0')
    {
        (*a)++;
        (*b)++;
    }
}

/**
  Zwraca czy słowo `a` można zamienić w `b` przez usunięcie znaku.
  @param[in] a Dłuższe słowo.
  @param[in] b Krótsze słowo.
  @return 1 jeśli się da zamienić `a` w `b` przez usunięcia znaku, 0 w p.p.
 */
static int can_transform_by_delete(const wchar_t *a, const wchar_t *b)
{
    skip_equal(&a, &b);
    a++;
    skip_equal(&a, &b);
    return *a == L'\0' && *b == L'\0';
}

/**
  Zwraca czy słowo `a` można zamienić w `b` przez zamianę znaku.
  @param[in] a Pierwsze słowo.
  @param[in] b Drugie słowo.
  @return 1 jeśli się da zamienić `a` w `b` przez zamianę znaku, 0 w p.p.
 */
static int can_transform_by_replace(const wchar_t *a, const wchar_t *b)
{
    skip_equal(&a, &b);
    a++; b++;
    skip_equal(&a, &b);
    return *a == L'\0' && *b == L'\0';
}

/**@}*/
/** @name Elementy interfejsu 
  @{
 */
struct dictionary * dictionary_new()
{
    struct dictionary *dict =
        (struct dictionary *) malloc(sizeof(struct dictionary));
    word_list_init(&dict->list);
    return dict;
}

void dictionary_done(struct dictionary *dict)
{
    dictionary_free(dict);
    free(dict);
}

int dictionary_insert(struct dictionary *dict, const wchar_t *word)
{
    if (dictionary_find(dict, word))
        return 0;
    word_list_add(&dict->list, word);
    return 1;
}

int dictionary_delete(struct dictionary *dict, const wchar_t *word)
{
    /// @bug `struct word_list` nie obsługuje operacji usuwania.
    return 0;
}

bool dictionary_find(const struct dictionary *dict, const wchar_t* word)
{
    const wchar_t * const * a = word_list_get(&dict->list);
    for (size_t i = 0; i < word_list_size(&dict->list); i++)
        if (!wcscmp(a[i], word))
            return true;
    return false;
}

int dictionary_save(const struct dictionary *dict, FILE* stream)
{
    const wchar_t * const * a = word_list_get(&dict->list);
    for (size_t i = 0; i < word_list_size(&dict->list); i++)
        if (fprintf(stream, "%ls\n", a[i]) < 0)
            return -1;
    return 0;
}

struct dictionary * dictionary_load(FILE* stream)
{
    struct dictionary *dict = dictionary_new();
    wchar_t buf[32];
    while (fscanf(stream, "%32ls", buf) != EOF)
        dictionary_insert(dict, buf);
    if (ferror(stream))
    {
        dictionary_done(dict);
        dict = NULL;
    }
    return dict;
}

void dictionary_hints(const struct dictionary *dict, const wchar_t* word,
        struct word_list *list)
{
    word_list_init(list);
    size_t wlen = wcslen(word);
    const wchar_t * const * a = word_list_get(&dict->list);
    for (size_t i = 0; i < word_list_size(&dict->list); i++)
    {
        size_t len = wcslen(a[i]);
        if (len == wlen - 1)
        {
            if (can_transform_by_delete(word, a[i]))
                word_list_add(list, a[i]);
        }
        else if (len == wlen)
        {
            if (can_transform_by_replace(word, a[i]))
                word_list_add(list, a[i]);
        }
        else if (len == wlen + 1)
        {
            if (can_transform_by_delete(a[i], word))
                word_list_add(list, a[i]);
        }
    }
}

/**@}*/
