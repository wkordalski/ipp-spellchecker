/** @file
    Interfejs podpowiedzi.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-02
 */

#ifndef DICTIONARY_RULE_H
#define DICTIONARY_RULE_H

/**
 * Reguła podpowiadania słów z drzewa.
 */
struct hint_rule;

#include "dictionary.h"
#include "list.h"
#include "trie.h"

struct hint_rule * rule_make(wchar_t *src, wchar_t *dst, int cost, enum rule_flag flag);

void rule_done(struct hint_rule *rule);

struct list * rule_generate_hints(struct hint_rule **rules, int max_cost, int max_hints_no, struct trie_node *root, const wchar_t *word);

#endif /* DICTIONARY_RULE_H */