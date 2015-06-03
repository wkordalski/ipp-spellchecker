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

static bool pattern_matches(const wchar_t *pattern, const wchar_t *text, wchar_t memory[10])
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
            if(memory[addr] == 0 || memory[addr] == 1)
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

static void mark_free_variables(wchar_t *replacement, wchar_t memory[10])
{
    if(*replacement >= L'0' && *replacement <= L'9')
    {
        int addr = *replacement - L'0';
        if(memory[addr] == 0)
        {
            memory[addr] = 1;
        }
    }
}

static bool is_specified(wchar_t replacement, wchar_t memory[10])
{
    if(iswalpha(replacement)) return true;
    int addr = *replacement - L'0';
    if(memory[addr] != 0 && memory[addr] != 1) return true;
    return false;
}

static wchar_t substitute_letter(wchar_t pattern, wchar_t memory[10])
{
    if(iswalpha(pattern)) return pattern;
    
}

static int rule_cost_sorter(void *a, void *b)
{
    struct hint_rule *A = *(struct hint_rule**)a;
    struct hint_rule *B = *(struct hint_rule**)b;
    return A->cost - B->cost;
}

// rule*[cost][idx]
static struct hint_rule *** preprocess_suffix(struct hint_rule **rules, int rcnt, wchar_t *word, bool begin)
{
    int max_cost = dictionary_hints_max_cost();
    
    // Sprawdzić, które reguły pasują
    struct hint_rule ** partial = malloc(rcnt * sizeof(struct hint_rule*));
    struct hint_rule ** partial_end = partial;
    
    for(struct hint_rule **i = rules; *i != NULL; i++)
    {
        wchar_t memory[10];
        if(pattern_matches((*i)->src, word, memory))
        {
            if((*i)->flag == RULE_BEGIN && begin == false) continue;
            if((*i)->flag == RULE_END && wcslen((*i)->src) != wcslen(word)) continue;
            *partial_end = *i;
            partial_end++;
        }
    }
    // Posortować reguły
    qsort(partial, partial_end - partial, sizeof(struct hint_rule*), rule_cost_sorter);
    //  Przepisać do kubełków?
    ///@todo Przepisać do kubełków
    struct hint_rule *** output = malloc((max_cost+1) * sizeof(struct hint_rule*));
    for(int i = 0; i < max_cost+1; i++)
    {
        //output[i] = malloc(sizeof(struct hint_rule **)
    }
}

// rule*[sufix][cost][idx]
static struct hint_rule **** preprocess(struct hint_rule **rules, wchar_t *word)
{
    int rcnt = 0;
    while(rule[rcnt] != NULL) rcnt++;
    int wlen = wcslen(word);
    struct hint_rule ****output = malloc(wlen * sizeof(struct hint_rule***));
    struct hint_rule ****output_walker = output;
    while(*word != 0)
    {
        *output_walker = preprocess_suffix(rules, rcnt, word);
        output_walker++;
        word++;
    }
    return output;
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