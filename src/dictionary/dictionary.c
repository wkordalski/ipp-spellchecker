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
#include "trie.h"
#include <stdio.h>
#include <stdlib.h>

#define _GNU_SOURCE

/**
  Struktura przechowująca słownik.
  Na razie prosta implementacja z użyciem listy słów.
 */
struct dictionary
{
    struct trie_node *root;      ///< Korzeń drzewa TRIE
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
    trie_done(dict->root);
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
    dict->root = trie_init();
    return dict;
}

void dictionary_done(struct dictionary *dict)
{
    dictionary_free(dict);
    free(dict);
}

int dictionary_insert(struct dictionary *dict, const wchar_t *word)
{
    return trie_insert(dict->root, word);
}

int dictionary_delete(struct dictionary *dict, const wchar_t *word)
{
    return trie_delete(dict->root, word);
}

bool dictionary_find(const struct dictionary *dict, const wchar_t* word)
{
    return trie_find(dict->root, word);
}

int dictionary_save(const struct dictionary *dict, FILE* stream)
{
    trie_serialize(dict->root, stream);
    return 0;
}

struct dictionary * dictionary_load(FILE* stream)
{
    struct trie_node * root = trie_deserialize(stream);
    if(root == NULL) return NULL;
    struct dictionary * dict = dictionary_new();
    trie_done(dict->root);
    dict->root = root;
    return dict;
}

void dictionary_hints(const struct dictionary *dict, const wchar_t* word,
        struct word_list *list)
{
    word_list_init(list);
    trie_hints(dict->root, word, list);
}

/**@}*/
