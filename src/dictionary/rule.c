/** @file
  Implementacja reguł podpowiedzi.

  @ingroup dictionary
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-06-03
 */

#include "dictionary.h"

#include <stdlib.h>
#include <wctype.h>

struct hint_rule
{
    wchar_t *src;               ///< Wzorzec do zastąpienia.
    wchar_t *dst;               ///< Tekst, którym zastąpić wzorzec.
    int cost;                   ///< Koszt użycia reguły.
    enum rule_flag flag;             ///< Flagi reguły.
};

struct state
{
    wchar_t *suf;
    struct trie_node * node;
    struct state *prnt;
    struct hint_rule *prev;
};

static bool pattern_matches(const wchar_t *pattern, const wchar_t *text, wchar_t *memory)
{
    for(int i = 0; i < 10; i++) memory[i] = 0;
    while(*pattern != 0)
    {
        if(iswalpha(*pattern))
        {
            if(*pattern != *text) return false;
            pattern++; text++;
        }
        else if(*pattern >= L'0' && *pattern <= L'9')
        {
            int addr = *pattern - L'0';
            if(memory[addr] == 0)
            {
                memory[addr] = *text;
                pattern++; text++;
            }
            else
            {
                if(memory[addr] != *text) return false;
                pattern++; text++;
            }
        }
        else return false;
    }
    return true;
}

struct hint_rule * rule_make(wchar_t *src, wchar_t *dst, int cost, enum rule_flag flag)
{
    struct hint_rule *rule = malloc(sizeof(struct hint_rule));
    rule->src = src;
    rule->dst = dst;
    rule->cost = cost;
    rule->flag = flag;
    return rule;
}

bool rule_matches(struct hint_rule *rule, const wchar_t *text)
{
    wchar_t memory[10];
    return pattern_matches(rule->src, text, memory);
}