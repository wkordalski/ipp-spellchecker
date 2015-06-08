/** @file
    Implementacja reguł podpowiedzi.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-03
 */

#include "dictionary.h"
#include "list.h"

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

static int state_sorter(void *a, void *b)
{
    struct state *A = *(state**)a;
    struct state *B = *(state**)b;
    if(A->node - B->node != 0) return A->node - B->node;
    int alen = wcslen(A->suf);
    int blen = wcslen(B->suf);
    return alen - blen;
}

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
            int addr = *pattern - L'0';void **oa = l->array;
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
    int addr = replacement - L'0';
    if(memory[addr] != 0 && memory[addr] != 1) return true;
    return false;
}

static wchar_t substitute_letter(wchar_t pattern, wchar_t memory[10])
{
    if(iswalpha(pattern)) return pattern;
    
}

static int rule_cost_sorter(const void *a, const void *b)
{
    struct hint_rule *A = *(struct hint_rule**)a;
    struct hint_rule *B = *(struct hint_rule**)b;
    return A->cost - B->cost;
}

// rule*[cost][idx]
static struct hint_rule *** preprocess_suffix(struct hint_rule **rules, int rcnt, int max_cost, wchar_t *word, bool begin)
{
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
    // Przepisać do kubełków
    int buckets_count = (*(partial_end-1))->cost+2;
    struct hint_rule *** output = malloc(buckets_count * sizeof(struct hint_rule*));
    output[buckets_count-1] = NULL;
    for(struct hint_rule **it = partial; it < partial_end; it++)
    {
        int cost = it[0]->cost;
        int bsize = 0;
        for(struct hint_rule **jt = it; jt < partial_end && jt[0]->cost == cost; jt++) bsize++;
        struct hint_rule **bucket = malloc((bsize + 1) * sizeof(struct hint_rule*));
        struct hint_rule **bucket_end = bucket;
        for(struct hint_rule **jt = it; jt < partial_end && jt[0]->cost == cost; jt++)
        {
            *bucket_end = *jt;
            bucket_end++;
        }
        *bucket_end = NULL;
        output[cost] = bucket;
    }
    free(partial);
    return output;
}

// rule*[sufix][cost][idx]
static struct hint_rule **** preprocess(struct hint_rule **rules, int max_cost, wchar_t *word)
{
    int rcnt = 0;
    while(rules[rcnt] != NULL) rcnt++;
    int wlen = wcslen(word);
    struct hint_rule ****output = malloc(wlen * sizeof(struct hint_rule***));
    struct hint_rule ****output_walker = output;
    bool begin = true;
    while(*word != 0)
    {
        *output_walker = preprocess_suffix(rules, rcnt, max_cost, word, begin);
        output_walker++;
        word++;
        begin = false;
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

// Add samecost new states to list of states l.
void rule_extend(struct list *l)
{
    list_sort_and_unify(l, state_sorter);
    ///@todo Implementacja.
}