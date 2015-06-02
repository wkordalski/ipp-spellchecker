/** @file
    Interfejs podpowiedzi.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-02
 */

#ifndef DICTIONARY_RULE_H
#define DICTIONARY_RULE_H

#include "dictionary.h"

#include <stdbool.h>

/**
 * Reguła podpowiadania słów z drzewa.
 */
struct hint_rule;

struct hint_rule * rule_make(wchar_t *src, wchar_t *dst, int cost, enum rule_flag flag);

bool rule_matches(struct hint_rule *rule, const wchar_t *text);

#endif /* DICTIONARY_RULE_H */