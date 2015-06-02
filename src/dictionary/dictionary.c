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
#include "conf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define _GNU_SOURCE

/**
  Struktura przechowująca słownik.
  
  Słowa są przechowywane w drzewie TRIE.
 */
struct dictionary
{
    struct trie_node *root;      ///< Korzeń drzewa TRIE
    ///@todo Dodać listę reguł
};

/** @name Funkcje pomocnicze
  @{
 */

int dict_file_filter(const struct dirent *f)
{
    if(f->d_type != DT_REG) return 0;
    int len = strlen(f->d_name);
    if(len < 5) return 0;
    if(strcmp(f->d_name + len - 5, ".dict")) return 0;
    return 1;
}

/**
 * @}
 */

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
    ///@todo Porawne wywoływanie trie_hints
    //trie_hints(dict->root, word, list);
}


int dictionary_lang_list(char **list, size_t *list_len)
{
    struct dirent **output = NULL;
    int r = scandir(CONF_PATH, &output, dict_file_filter, alphasort);
    if(r < 0) return r;
    if(output == NULL)
    {
        *list = malloc(sizeof(char));
        (*list)[0] = 0;
        *list_len = 1;
        return 0;
    }
    size_t slen = 0;
    for(int i = 0; output[i] != NULL; i++)
    {
        slen += strlen(output[i]->d_name) + 1-5;
    }
    if(slen == 0)
    {
        *list = malloc(sizeof(char));
        (*list)[0] = 0;
        *list_len = 1;
        return 0;
    }
    *list = malloc(sizeof(char)*slen);
    memset(*list, 0, slen);
    size_t clen = 0;
    for(int i = 0; output[i] != NULL; i++)
    {
        size_t len = strlen(output[i]->d_name) - 5;
        memcpy(*list + clen, output[i]->d_name, len);
        clen += len + 1;
    }
    *list_len = slen;
    return 1;
}

struct dictionary * dictionary_load_lang(const char *lang)
{
    int cp_len = strlen(CONF_PATH);
    int lg_len = strlen(lang);
    char * fname = malloc((cp_len+1+lg_len+5+1)*sizeof(char));
    memcpy(fname, CONF_PATH, cp_len);
    fname[cp_len] = '/';
    memcpy(fname+cp_len+1, lang, lg_len);
    memcpy(fname+cp_len+1+lg_len, ".dict", 6);
    FILE * f = fopen(fname, "w");
    if(f == NULL) return NULL;
    struct dictionary *r = dictionary_load(f);
    fclose(f);
    free(fname);
    return r;
}

int dictionary_save_lang(const struct dictionary *dict, const char *lang)
{
    mkdir(CONF_PATH, S_IRWXU);
    int cp_len = strlen(CONF_PATH);
    int lg_len = strlen(lang);
    char * fname = malloc((cp_len+1+lg_len+5+1)*sizeof(char));
    memcpy(fname, CONF_PATH, cp_len);
    fname[cp_len] = '/';
    memcpy(fname+cp_len+1, lang, lg_len);
    memcpy(fname+cp_len+1+lg_len, ".dict", 6);
    FILE * f = fopen(fname, "w");
    int r = dictionary_save(dict, f);
    fclose(f);
    free(fname);
    return r;
}

/**@}*/
