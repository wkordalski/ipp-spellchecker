/** @file
    Implementacja reguł podpowiedzi.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-03
 */

#include "dictionary.h"
#include "list.h"
#include "trie.h"

#include <assert.h>
#include <stdlib.h>
#include <wctype.h>

#include "../testable.h"

struct hint_rule
{
    wchar_t *src;               ///< Wzorzec do zastąpienia.
    wchar_t *dst;               ///< Tekst, którym zastąpić wzorzec.
    int cost;                   ///< Koszt użycia reguły.
    enum rule_flag flag;        ///< Flagi reguły.
};

struct state
{
    const wchar_t *suf;               ///< Sufiks do poprawienia
    const struct trie_node * node;    ///< Aktualny węzeł w słowniku
    const struct trie_node * prev;    ///< NULL jeśli nie ma poprzedniego słowa lub wskaźnik na poprzednie słowo
    struct state *prnt;         ///< Poprzedni stan
    struct hint_rule *rule;     ///< Reguła wykorzystana do przejścia z poprzedniego do aktualnego stanu.
    wchar_t free_variable;      ///< Wartość po prawej stronie reguły, która mogła być dowolna.
};

struct costed_state
{
    struct state *s;
    int cost;
};

static int state_sorter(const void *a, const void *b)
{
    struct state *A = *(struct state**)a;
    struct state *B = *(struct state**)b;
    if((size_t)A->node - (size_t)B->node != 0) return (size_t)A->node - (size_t)B->node;
    if((size_t)A->prev - (size_t)B->prev != 0) return (size_t)A->prev - (size_t)B->prev;
    if(A->rule->flag == RULE_END && B->rule->flag != RULE_END) return 1;
    if(A->rule->flag != RULE_END && B->rule->flag == RULE_END) return -1;
    int alen = wcslen(A->suf);
    int blen = wcslen(B->suf);
    return alen - blen;
}

static int costed_state_sorter(const void *a, const void *b)
{
    struct costed_state *A = *(struct costed_state**)a;
    struct costed_state *B = *(struct costed_state**)b;
    int cmp = state_sorter(&(A->s), &(B->s));
    if(cmp != 0) return cmp;
    else return A->cost - B->cost;
}

static int costed_state_comparer(const void *a, const void *b)
{
    struct costed_state *A = *(struct costed_state**)a;
    struct costed_state *B = *(struct costed_state**)b;
    return state_sorter(&(A->s), &(B->s));
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
        else assert(false);
    }
    return true;
}

static wchar_t translate_letter(wchar_t c, wchar_t memory[10])
{
    if(iswalpha(c)) return c;
    if(c >= L'0' && c <= L'9')
    {
        if(memory[c-L'0'] == 0) return c - L'0';
        return memory[c - L'0'];
    }
    return -1;
}

static int rule_cost_sorter(const void *a, const void *b)
{
    struct hint_rule *A = *(struct hint_rule**)a;
    struct hint_rule *B = *(struct hint_rule**)b;
    return A->cost - B->cost;
}

// rule*[cost][idx]
static struct hint_rule *** preprocess_suffix(struct hint_rule **rules, int rcnt, const wchar_t *word, bool begin)
{
    // Sprawdzić, które reguły pasują
    struct hint_rule ** partial = malloc(rcnt * sizeof(struct hint_rule*));
    struct hint_rule ** partial_end = partial;
    
    for(int i = 0; i < rcnt; i++)
    {
        struct hint_rule *it = rules[i];
        wchar_t memory[10];
        if(pattern_matches(it->src, word, memory))
        {
            if(it->flag == RULE_BEGIN && begin == false) continue;
            if(it->flag == RULE_END && wcslen(it->src) != wcslen(word)) continue;
            *partial_end = it;
            partial_end++;
        }
    }
    if(partial == partial_end)
    {
        free(partial);
        struct hint_rule *** output = malloc(sizeof(struct hint_rule**));
        output[0] = NULL;
        return output;
    }
    // Posortować reguły
    qsort(partial, partial_end - partial, sizeof(struct hint_rule*), rule_cost_sorter);
    // Przepisać do kubełków
    int buckets_count = (*(partial_end-1))->cost+2;
    struct hint_rule *** output = malloc(buckets_count * sizeof(struct hint_rule*));
    for(int i = 0; i < buckets_count; i++) output[i] = NULL;
    for(struct hint_rule **it = partial; it < partial_end;)
    {
        int cost = it[0]->cost;
        int bsize = 0;
        for(struct hint_rule **jt = it; jt < partial_end && jt[0]->cost == cost; jt++) bsize++;
        struct hint_rule **bucket = malloc((bsize + 1) * sizeof(struct hint_rule*));
        struct hint_rule **bucket_end = bucket;
        for(; it < partial_end && it[0]->cost == cost; it++)
        {
            *bucket_end = *it;
            bucket_end++;
        }
        *bucket_end = NULL;
        output[cost] = bucket;
    }
    // Last one should be simply NULL!
    for(int i = 0; i < buckets_count - 1; i++)
    {
        if(output[i] == NULL)
        {
            output[i] = malloc(sizeof(struct hint_rule*));
            output[i][0] = NULL;
        }
    }
    free(partial);
    return output;
}

// rule*[sufix][cost][idx]
static struct hint_rule **** preprocess(struct hint_rule **rules, const wchar_t *word)
{
    int rcnt = 0;
    while(rules[rcnt] != NULL) rcnt++;
    int wlen = wcslen(word);
    struct hint_rule ****output = malloc((wlen+1) * sizeof(struct hint_rule***));
    struct hint_rule ****output_walker = output + wlen;
    bool begin = true;
    while(*word != 0)
    {
        *output_walker = preprocess_suffix(rules, rcnt, word, begin);
        output_walker--;
        word++;
        begin = false;
    }
    *output = preprocess_suffix(rules, rcnt, word, begin);  // last one...
    return output;
}

static void free_preprocessing_data_for_suffix(struct hint_rule ***pp)
{
    for(struct hint_rule ***it = pp; *it != NULL; it++)
    {
        free(*it);
    }
    free(pp);
}

static void free_preprocessing_data(struct hint_rule ****pp, int wlen)
{
    for(int i = 0; i <= wlen; i++)
    {
        free_preprocessing_data_for_suffix(pp[i]);
    }
    free(pp);
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

void rule_done(struct hint_rule *rule)
{
    free(rule);
}


static struct list * extend_state(struct state *s)
{
    struct list * ret = list_init();
    list_add(ret, s);
    while(1)
    {
        if(s->suf[0] == 0) return ret;
        const struct trie_node *nn = trie_get_child(s->node, s->suf[0]);
        if(nn == NULL) return ret;
        struct state *ns = malloc(sizeof(struct state));
        ns->prnt = s;
        ns->rule = NULL; // Special rule for extending :P
        ns->suf = s->suf + 1;
        ns->node = nn;
        ns->prev = s->prev;
        list_add(ret, ns);
        s = ns;
    }
}

static void explore_trie(const struct trie_node *n,
                         wchar_t *dst,
                         wchar_t memory[10],
                         struct list *l,
                         struct state *ps,
                         struct hint_rule *r,
                         const wchar_t *suf,
                         const struct trie_node *root,
                         wchar_t last_guessed)
{
    if(*dst == 0)
    {
        struct state *s = malloc(sizeof(struct state));
        s->node = n;
        s->prev = ps->prev;
        s->rule = r;
        s->prnt = ps;
        s->suf = suf;
        s->free_variable = last_guessed;
        if(r->flag == RULE_SPLIT || r->flag == RULE_END)
        {
            if(!trie_is_leaf(n))
            {
                free(s);
                return;
            }
        }
        if(r->flag == RULE_SPLIT)
        {
            if(s->prev != NULL)
            {
                free(s);
                return;
            }
            s->prev = s->node;
            s->node = root;
        }
        list_add_list(l, extend_state(s));
        return;
    }
    wchar_t addtn = translate_letter(*dst, memory);
    if(addtn == -1) return;
    if(addtn >= 0 && addtn <= 9)
    {
        const struct trie_node **nodes;
        int cnt = trie_get_children(n, &nodes);
        for(int i = 0; i < cnt; i++)
        {
            const struct trie_node *curr = nodes[i];
            memory[addtn] = trie_get_value(curr);
            explore_trie(curr, dst+1, memory, l, ps, r, suf, root, trie_get_value(curr));
        }
        memory[addtn] = 0;
    }
    else
    {
        const struct trie_node *curr = trie_get_child(n, addtn);
        if(curr == NULL) return;
        explore_trie(curr, dst+1, memory, l, ps, r, suf, root, last_guessed);
    }
}

static struct list * apply_rule(struct state *s, struct hint_rule *r, const struct trie_node *root)
{
    wchar_t memory[10];
    struct list *ret = list_init();
    if(!pattern_matches(r->src, s->suf, memory)) return NULL;
    explore_trie(s->node, r->dst, memory, ret, s, r, s->suf + wcslen(r->src), root, 0);
    return ret;
}

static struct list * apply_rules_to_states(struct list *s, int c, const struct trie_node *root, struct hint_rule ****pp)
{
    struct state ** sts = (struct state**)list_get(s);
    struct list *ret = list_init();
    for(int i = 0; i < list_size(s); i++)
    {
        struct list *std = list_init();
        struct state *ss = sts[i];
        if(ss->rule != NULL && ss->rule->flag == RULE_END) continue;
        struct hint_rule ***rg = pp[wcslen(ss->suf)];
        bool hasc = true;
        for(int i = 0; i < c; i++)
        {
            if(rg[i] == NULL) hasc = false;
        }
        if(hasc)
        {
            struct hint_rule **hrg = pp[wcslen(ss->suf)][c];
            if(hrg != NULL)
            {
                while(*hrg != NULL)
                {
                    struct list *lst = apply_rule(ss, *hrg, root);
                    list_add_list(std, lst);
                    list_done(lst);
                    hrg++;
                }
            }
        }
        list_add_list(ret, std);
        list_done(std);
    }
    return ret;
}

static void unify_states(struct list **ll, int mc)
{
    struct list *ret = list_init();
    int sno = 0;
    for(int i = 0; i <= mc; i++) sno += list_size(ll[i]);
    list_reserve(ret, sno + 4);
    for(int i = 0; i <= mc; i++)
    {
        struct state **ss = (struct state **)list_get(ll[i]);
        for(int j = 0; j < list_size(ll[i]); j++)
        {
            struct costed_state *cs = malloc(sizeof(struct costed_state));
            cs->cost = i;
            cs->s = ss[j];
            list_add(ret, cs);
        }
    }
    struct list *dups = list_init();
    list_sort_and_unify(ret, costed_state_sorter, costed_state_comparer, dups);
    for(int i = 0; i <= mc; i++)
    {
        list_clear(ll[i]);
    }
    struct costed_state **css = (struct costed_state **)list_get(ret);
    for(int i = 0; i < list_size(ret); i++)
    {
        struct costed_state *cs = css[i];
        list_add(ll[cs->cost], cs->s);
        free(cs);
    }
    css = (struct costed_state **)list_get(dups);
    for(int i = 0; i < list_size(dups); i++)
    {
        struct costed_state *cs = css[i];
        free(cs->s);
        free(cs);
    }
    list_done(dups);
    list_done(ret);
}

static int locale_sorter(const void *a, const void *b)
{
    return wcscoll(*(const wchar_t**)a, *(const wchar_t**)b);
}

static void get_text_helper(struct state *s, struct list *l, const struct trie_node *prev)
{
    if(s->prnt == NULL) return;
    get_text_helper(s->prnt, l, prev);
        
    if(s->rule == NULL)
    {
        list_add(l, (void*)trie_get_value_ptr(s->node));
    }
    else
    {
        wchar_t memory[10];
        pattern_matches(s->rule->src, s->prnt->suf, memory);
        wchar_t *dst = s->rule->dst;
        for(int i = 0; i < 10; i++) if(memory[i] == 0) memory[i] = s->free_variable;
        const struct trie_node *on = s->prnt->node;
        while(*dst != 0)
        {
            wchar_t val = translate_letter(*dst, memory);
            on = trie_get_child(on, val);
            list_add(l, (void*)trie_get_value_ptr(on));
            dst++;
        }
    }
    if(s->node == prev) list_add(l, NULL);  // Spacja
}

static wchar_t * get_text(struct state *s)
{
    struct list *l = list_init();
    get_text_helper(s, l, s->prev);
    wchar_t *rt = malloc((list_size(l)+1)*sizeof(wchar_t));
    wchar_t **ls = (wchar_t**)list_get(l);
    for(int i = 0; i < list_size(l); i++)
    {
        if(ls[i] == NULL) rt[i] = L' ';
        else rt[i] = *(ls[i]);
    }
    rt[list_size(l)] = 0;
    list_done(l);
    return rt;
}

static int text_sorter(void *a, void *b)
{
    wchar_t *A = *(wchar_t**)a;
    wchar_t *B = *(wchar_t**)b;
    
    while(*A == *B && *A != 0)
    {
        A++; B++;
    }
    return *A - *B;
}

struct list * rule_generate_hints(struct hint_rule **rules, int max_cost, int max_hints_no, struct trie_node *root, const wchar_t *word)
{
    int wlen = wcslen(word);
    struct hint_rule ****pp = preprocess(rules, word);
    struct state *is = malloc(sizeof(struct state));
    is->node = root;
    is->prev = NULL;
    is->prnt = NULL;
    is->rule = NULL;
    is->suf = word;
    struct list **layers = malloc(sizeof(struct list*)*(max_cost+1));
    layers[0] = extend_state(is);
    for(int i = 1; i <= max_cost; i++)
    {
        layers[i] = list_init();
    }
    struct list *output = list_init();
    struct list *so = list_init();  // Sorted output
    struct state **l0 = (struct state**)list_get(layers[0]);
    for(int i = 0; i < list_size(layers[0]); i++)
    {
        if(l0[i]->suf[0] == 0 && trie_is_leaf(l0[i]->node))
        {
            // stan końcowy
            list_add(output, get_text(l0[i]));
        }
    }
    list_add_list(so, output);
    list_sort(so, text_sorter);
    for(int i = 1; i <= max_cost; i++)
    {
        struct list *po = list_init();
        for(int j = 1; j <= i; j++)
        {
            int lno = i - j;
            list_add_list(layers[i], apply_rules_to_states(layers[lno], j, root, pp));
        }
        unify_states(layers, i);
        struct state **li = (struct state **)list_get(layers[i]);
        for(int j = 0; j < list_size(layers[i]); j++)
        {
            if(li[j]->suf[0] == 0 && trie_is_leaf(li[j]->node))
            {
                // stan końcowy
                list_add(po, get_text(li[j]));
            }
        }
        list_sort_and_unify(po, locale_sorter, locale_sorter, NULL);
        struct list *toadd = list_init();
        list_reserve(toadd, list_size(po));
        for(int i = 0; i < list_size(po); i++)
        {
            const wchar_t *e = (const wchar_t*)list_get(po)[i];
            const wchar_t **t = (const wchar_t**)list_get(so);
            wchar_t **r =  bsearch(&e, t, list_size(so), sizeof(const wchar_t*), text_sorter);
            if(r == NULL)
                list_add(toadd, e);
            else
                free(e);
        }
        list_add_list(output, toadd);
        list_add_list(so, toadd);
        list_sort(so, text_sorter);
        list_done(toadd);
        list_done(po);
        if(list_size(output) >= max_hints_no) goto done;
    }
done:
    // Clean-up!
    for(int i = 0; i < max_cost; i++)
    {
        struct list *ll = layers[i];
        struct state **ss = (struct state**)list_get(ll);
        for(int j = 0; j < list_size(ll); j++)
        {
            free(ss[j]);
        }
        list_done(ll);
    }
    free(layers);
    list_done(so);
    // Preprocessing data
    free_preprocessing_data(pp, wlen);
    if(list_size(output) > max_hints_no)
        list_resize(output, max_hints_no, NULL);
    return output;
}

