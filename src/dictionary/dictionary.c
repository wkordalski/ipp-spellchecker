/** @file
  Prosta implementacja słownika.
  Słownik przechowuje drzewo TRIE.
  
  @ingroup dictionary
  @author Jakub Pawlewicz <pan@mimuw.edu.pl>
          Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-06-15
 */

#include "conf.h"
#include "dictionary.h"
#include "list.h"
#include "rule.h"
#include "trie.h"

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
    int max_cost;                ///< Maksymalny koszt podpowiedzi.
    struct list *rules;          ///< Lista reguł podpowiedzi.
};

/** @name Funkcje pomocnicze
  @{
 */

/**
 * Filters only files with .dict extension.
 * @param[in] f Filter entity.
 * @return 1 if accept file, 0 otherwise.
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
 * Simple wrapper for rule_done for list_iter.
 * @param[in] r Rule to free.
 * @param[in] ctx Useless context.
 */
void rule_done_wrapper(void *r, void *ctx)
{
    if(r != NULL)
        rule_done((struct hint_rule*)r);
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
    dict->rules = list_init();
    dict->max_cost = 0;
    return dict;
}

void dictionary_done(struct dictionary *dict)
{
    trie_done(dict->root);
    list_iter(dict->rules, NULL, rule_done_wrapper);
    list_done(dict->rules);
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
    if(trie_serialize(dict->root, stream)<0) return -1;
    if(list_serialize(dict->rules, stream, rule_serialize)<0) return -1;
    if(fputwc(dict->max_cost, stream)<0) return -1;
    return 0;
}

struct dictionary * dictionary_load(FILE* stream)
{
    struct trie_node * root = trie_deserialize(stream);
    if(root == NULL) goto fail;
    struct list *rules = list_deserialize(stream, rule_deserialize);
    if(rules == NULL) goto fail;
    int mcost = fgetwc(stream);
    if(mcost < 0) goto fail;
    struct dictionary * dict = 
        (struct dictionary *) malloc(sizeof(struct dictionary));
    dict->root = root;
    dict->rules = rules;
    dict->max_cost = mcost;
    return dict;
fail:
    if(root != NULL) trie_done(root);
    if(rules != NULL)
    {
        list_iter(rules, NULL, rule_done_wrapper);
        list_done(rules);
    }
    return NULL;
}

void dictionary_hints(const struct dictionary *dict, const wchar_t* word,
        struct word_list *list)
{
    word_list_init(list);
    trie_hints(dict->root, word, list, dict->rules, dict->max_cost, DICTIONARY_MAX_HINTS);
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
    FILE * f = fopen(fname, "r");
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

void dictionary_rule_clear(struct dictionary* dict)
{
    list_clear(dict->rules);
}

int dictionary_rule_add(struct dictionary* dict, const wchar_t* left, const wchar_t* right, bool bidirectional, int cost, enum rule_flag flag)
{
    struct hint_rule *r = rule_make(left, right, cost, flag);
    int ret = 1;
    if(r != NULL) list_add(dict->rules, r);
    else ret = 0;
    if(bidirectional) ret += dictionary_rule_add(dict, right, left, false, cost, flag);
    return ret;
}

int dictionary_hints_max_cost(struct dictionary* dict, int new_cost)
{
    int r = dict->max_cost;
    dict->max_cost = new_cost;
    return r;
}


/**@}*/
