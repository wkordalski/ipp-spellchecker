/** @file
  Prosta implementacja słownika.
  Słownik przechowuje drzewo TRIE.
  
  @ingroup dictionary
  @author Jakub Pawlewicz <pan@mimuw.edu.pl>
          Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-05-23
 */

#include "dictionary.h"
#include "trie.h"
#include <stdio.h>
#include <stdlib.h>

#define _GNU_SOURCE

/**
  Struktura przechowująca słownik.
  
  Słowa są przechowywane w drzewie TRIE.
 */
struct dictionary
{
    struct trie_node *root;      ///< Korzeń drzewa TRIE
};

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
    trie_done(dict->root);
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
    struct dictionary * dict = 
        (struct dictionary *) malloc(sizeof(struct dictionary));
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
