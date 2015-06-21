/** @file
    Implementacja reguł podpowiedzi.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-15
 */

#include "dictionary.h"
#include "list.h"
#include "serialization.h"
#include "str.h"
#include "trie.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "../testable.h"

/**
 * Reguła podpowiadania słów z drzewa.
 */
struct hint_rule
{
    wchar_t *src;               ///< Wzorzec do zastąpienia.
    wchar_t *dst;               ///< Tekst, którym zastąpić wzorzec.
    int cost;                   ///< Koszt użycia reguły.
    enum rule_flag flag;        ///< Flagi reguły.
};

/**
 * Stan algorytmu generującego podpowiedzi.
 */
struct state
{
    const wchar_t *suf;               ///< Sufiks do poprawienia
    const struct trie_node * node;    ///< Aktualny węzeł w słowniku
    const struct trie_node * prev;    ///< NULL jeśli nie ma poprzedniego słowa lub wskaźnik na poprzednie słowo
    struct state *prnt;               ///< Poprzedni stan
    struct hint_rule *rule;           ///< Reguła wykorzystana do przejścia z poprzedniego do aktualnego stanu.
    wchar_t free_variable;            ///< Wartość po prawej stronie reguły, która mogła być dowolna.
};

/**
 * Stan algorytmu generującego podpowiedzi uzupełniony o informację o aktualnym koszcie
 */
struct costed_state
{
    struct state *s;                    ///< Stan bazowy
    int cost;                           ///< Koszt stanu
};

/** @name Funkcje pomocnicze
 * @{
 */

/**
 * Porządek częściowy na stanach.
 * @param[in] a Wskaźnik na wskaźnik na pierwszy stan.
 * @param[in] b Wskaźnik na wskaźnik na drugi stan.
 * @return 0 jeśli są równe, liczbę dodatnią lub ujemną
 * jeśli odpowiednio pierwszy stan jest większy od drugiego lub drugi od pierwszego.
 */
static int state_sorter(const void *a, const void *b)
{
    struct state *A = *(struct state**)a;
    struct state *B = *(struct state**)b;
    if((size_t)A->node - (size_t)B->node != 0) return (size_t)A->node - (size_t)B->node;
    if((size_t)A->prev - (size_t)B->prev != 0) return (size_t)A->prev - (size_t)B->prev;
    if((A->rule != NULL && A->rule->flag == RULE_END) && (B->rule == NULL || B->rule->flag != RULE_END)) return 1;
    if((A->rule == NULL || A->rule->flag != RULE_END) && (B->rule != NULL && B->rule->flag == RULE_END)) return -1;
    int alen = wcslen(A->suf);
    int blen = wcslen(B->suf);
    return alen - blen;
}

/**
 * Porządek częściowy na stanach wzbogaconych o koszty.
 * Sortujemy najpierw po stanie bazowym, a potem po koszcie.
 * 
 * @param[in] a Wskaźnik na wskaźnik na pierwszy stan.
 * @param[in] b Wskaźnik na wskaźnik na drugi stan.
 * @return 0 jeśli są równe, liczbę dodatnią lub ujemną jeśli
 * odpowiednio pierwszy stan jest większy od drugiego lub drugi od pierwszego.
 */
static int costed_state_sorter(const void *a, const void *b)
{
    struct costed_state *A = *(struct costed_state**)a;
    struct costed_state *B = *(struct costed_state**)b;
    int cmp = state_sorter(&(A->s), &(B->s));
    if(cmp != 0) return cmp;
    else return A->cost - B->cost;
}

/**
 * Porównuje stany wzbogacone o koszty.
 * Stany te są równe jeśli stany bazowe są równe.
 * Czyli wywołanie state_sorter zwraca 0.
 * 
 * @param[in] a Wskaźnik na wskaźnik na pierwszy stan.
 * @param[in] b Wskaźnik na wskaźnik na drugi stan.
 * @return 0 jeśli są równe, cokolwiek niezerowego w p.p.
 */
static int costed_state_comparer(const void *a, const void *b)
{
    struct costed_state *A = *(struct costed_state**)a;
    struct costed_state *B = *(struct costed_state**)b;
    return state_sorter(&(A->s), &(B->s));
}

/**
 * Próbuje przypasować wzorzec do tekstu.
 * 
 * @param[in] pattern Wzorzec.
 * @param[in] text Tekst.
 * @param[in,out] memory Pamięć, gdzie zapisać przypisania wartości do zmiennych.
 * @return True jeśli udało się dopasować, false w p.p.
 */
static bool pattern_matches(const wchar_t *pattern, const wchar_t *text, wchar_t memory[10])
{
    for(int i = 0; i < 10; i++) memory[i] = 0;
    while(*pattern != 0)
    {
        if(*text == 0) return false;
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
        else assert(false);
    }
    return true;
}

/**
 * Zamienia literę z tekstu zastępczego na właściwą literę uwzględniając zmienne.
 * 
 * @param[in] c Znak z tekstu zastępczego.
 * @param[in] memory Pamięć z przypisaniem zmiennych.
 * @return Wynikowa litera.
 */
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

/**
 * Porządek częściowy na regułach do sortowania po koszcie.
 * 
 * @param[in] a Wskaźnik na wskaźnik na pierwszą regułę.
 * @param[in] b Wskaźnik na wskaźnik na drugą regułę.
 * @return 0 jeśli koszty są równe, liczba dodatnia lub ujemna
 * jeśli odpowiednio pierwsza reguła jest droższa od drugiej
 * lub druga od pierwszej.
 */
static int rule_cost_sorter(const void *a, const void *b)
{
    struct hint_rule *A = *(struct hint_rule**)a;
    struct hint_rule *B = *(struct hint_rule**)b;
    return A->cost - B->cost;
}

/**
 * Wykonuje preprocessing przed generacją podpowiedzi dla danego sufiksu.
 * 
 * @param[in] rules NULL-terminated lista wskaźników na reguły.
 * @param[in] rcnt Liczba reguł na liście.
 * @param[in] word Sufiks.
 * @param[in] begin Czy sufiks jest całym słowem do wygenerowania podpowiedzi.
 * @return Wskaźnik na tablicę list wskaźników na reguły.
 * Tablica jest indeksowana po kosztach reguł.
 */
static struct list * preprocess_suffix(struct hint_rule **rules, int rcnt, const wchar_t *word, bool begin)
{
    // Sprawdzić, które reguły pasują
    struct list *ret = list_init();
    list_reserve(ret, rcnt);    // So O(1) additions only
    
    for(int i = 0; i < rcnt; i++)
    {
        struct hint_rule *it = rules[i];
        wchar_t memory[10];
        if(pattern_matches(it->src, word, memory))
        {
            if(it->flag == RULE_BEGIN && begin == false) continue;
            if(it->flag == RULE_END && wcslen(it->src) != wcslen(word)) continue;
            list_add(ret, it);
        }
    }
    if(list_size(ret) == 0)
    {
        list_reserve(ret, 0);   // Shrink maximally to save memory
        return ret;
    }
    // Posortować reguły
    list_sort(ret, rule_cost_sorter);
    list_reserve(ret, 0);
    return ret;
}

/**
 * Wykonuje preprocessing dla danego słowa.
 * 
 * @param[in] rules NULL-terminated lista reguł.
 * @param[in] word Słowo do wygenerowania podpowiedzi.
 * @return Wskaźnik na dwuwymiarową tablicę list wskaźników na reguły.
 * Tablica jest indeksowana po 1. długości sufiksu, 2. koszcie reguł.
 */
static struct list ** preprocess(struct hint_rule **rules, const wchar_t *word)
{
    int rcnt = 0;
    while(rules[rcnt] != NULL) rcnt++;
    int wlen = wcslen(word);
    struct list **output = malloc((wlen+1) * sizeof(struct list*));
    struct list **output_walker = output + wlen;
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

/**
 * Zwalnia pamięć zaalokowaną przez preprocess_suffix.
 * 
 * @param[in] pp Wskaźnik zwrócony przez tą funkcję.
 */
static void free_preprocessing_data_for_suffix(struct list *pp)
{
    list_done(pp);
}

/**
 * Zwalnia pamięć zaalokowaną przez preprocess.
 * 
 * @param[in] pp Wskaźnik zwrócony przez tą funkcję.
 * @param[in] wlen Długość słowa, dla którego szukaliśmy podpowiedzi.
 */
static void free_preprocessing_data(struct list **pp, int wlen)
{
    for(int i = 0; i <= wlen; i++)
    {
        free_preprocessing_data_for_suffix(pp[i]);
    }
    free(pp);
}

/**
 * Dodaje stany pochodne bez użycia reguł.
 * 
 * @param[in] s Stan do rozwinięcia.
 * @return Lista stanów pochodnych o tym samym koszcie.
 */
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

/**
 * Próbuje zaaplikować regułę do stanu. Funkcja pomocnicza.
 * 
 * @param[in] n Aktualny węzeł drzewa TRIE.
 * @param[in] dst Sufiks tekstu zastępczego.
 * @param[in,out] memory Pamięć dla zmiennych w regule.
 * @param[in,out] l Lista do której dodać uzyskane stany.
 * @param[in] ps Stan źródłowy.
 * @param[in] r Zastosowana reguła.
 * @param[in] suf Sufiks tekstu do zastąpienia.
 * @param[in] root Korzeń drzewa TRIE.
 * @param[in] last_guessed Wartość ostatniej wolnej zmiennej.
 */
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
        list_add_list_and_free(l, extend_state(s));
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

/**
 * Próbuje zaaplikować regułę do stanu.
 * 
 * @param[in] s Stan.
 * @param[in] r Reguła.
 * @param[in] root Korzeń drzewa TRIE.
 * @return Lista stanów pochodnych.
 */
static struct list * apply_rule(struct state *s, struct hint_rule *r, const struct trie_node *root)
{
    wchar_t memory[10];
    struct list *ret = list_init();
    if(!pattern_matches(r->src, s->suf, memory)) return NULL;
    explore_trie(s->node, r->dst, memory, ret, s, r, s->suf + wcslen(r->src), root, 0);
    return ret;
}

/**
 * Znajduje reguły o danym koszcie na posortowanej liście.
 * 
 * @param[in] c Koszt.
 * @param[in] l Posortowana lista.
 * @param[out] o Wskaźnik na pierwszą regułę o danym koszcie.
 * @param[out] s Liczba elementów o danym koszcie.
 */
void find_rules_with_cost(int c, struct list *l, struct hint_rule ***o, int *s)
{
    struct hint_rule *mock = rule_make(L"a", L"b", c, RULE_NORMAL);
    struct hint_rule **lp = (struct hint_rule**)list_get(l);
    struct hint_rule **lq = lp + list_size(l);
    struct hint_rule **lb = bsearch(&mock, lp, list_size(l), sizeof(void*), rule_cost_sorter);
    struct hint_rule **le = lb;
    rule_done(mock);
    if(lb == NULL)
    {
        *s = 0;
        *o = NULL;
        return;
    }
    while(le < lq && le[0]->cost == c) le++;
    while(lb > lp && lb[-1]->cost == c) lb--;
    *o = lb;
    *s = le - lb;
}

/**
 * Próbuje zaaplikować reguły do stanów.
 * 
 * @param[in] s Lista stanów.
 * @param[in] c Koszt reguły do zastosowania.
 * @param[in] root Korzeń drzewa TRIE.
 * @param[in] pp Wynik preprocessingu.
 * @param[in] begin Stan początkowy.
 * @return Lista stanów pochodnych.
 */
static struct list * apply_rules_to_states(struct list *s, int c, const struct trie_node *root, struct list **pp, struct state *begin)
{
    struct state ** sts = (struct state**)list_get(s);
    struct list *ret = list_init();
    for(int i = 0; i < list_size(s); i++)
    {
        struct list *std = list_init();
        struct state *ss = sts[i];
        if(begin != ss  && ss->rule != NULL && ss->rule->flag == RULE_BEGIN)
        {
            list_done(std);
            continue;
        }
        if(ss->rule != NULL && ss->rule->flag == RULE_END)
        {
            list_done(std);
            continue;
        }
        // Find specific rules (filter costs) in pp
        struct list *rg = pp[wcslen(ss->suf)];
        struct hint_rule **rs = NULL;
        int rl = 0;
        find_rules_with_cost(c, rg, &rs, &rl);
        for(int i = 0; i < rl; i++, rs++)
        {
            list_add_list_and_free(std, apply_rule(ss, *rs, root));
        }
        list_add_list_and_free(ret, std);
    }
    return ret;
}

/**
 * Usuwa duplikaty stanów.
 * 
 * @param[in,out] ll Tablica list stanów. Każda lista przechowuje stany o innej wartości.
 * @param[in] mc Maksymalny koszt jaki uwzględnić.
 */
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

/**
 * Porównuje w-stringi alfabetycznie.
 * 
 * @param[in] a Wskaźnik na pierwszy tekst.
 * @param[in] b Wskaźnik na drugi tekst.
 * @return 0 jeśli są równe, liczbę dodatnią lub ujemną
 * jeśli odpowiednio pierwszy tekst jest większy od drugiego lub drugi od pierwszego.
 */
static int locale_sorter(const void *a, const void *b)
{
    return wcscoll(*(const wchar_t**)a, *(const wchar_t**)b);
}

/**
 * Oblicza tekst wygenerowany przez stan. Funkcja pomocnicza.
 * 
 * @param[in] s Aktualny stan.
 * @param[in] l Lista, gdzie zapisać wynik.s
 */
static void get_text_helper(struct state *s, struct list *l)
{
    if(s->prnt == NULL) return;
    get_text_helper(s->prnt, l);
    
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
    // when used split rule -> add space
    if(s->rule != NULL && s->rule->flag == RULE_SPLIT) list_add(l, NULL);
}

/**
 * Oblicza tekst wygenerowany przez stan. Funkcja pomocnicza.
 * 
 * @param[in] s Stan.
 * @return Tekst.
 */
static wchar_t * get_text(struct state *s)
{
    struct list *l = list_init();
    get_text_helper(s, l);
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

/**
 * Porównuje w-stringi po wartościach znaków.
 * 
 * @param[in] a Wskaźnik na pierwszy tekst.
 * @param[in] b Wskaźnik na drugi tekst.
 * @return 0 jeśli są równe, liczbę dodatnią lub ujemną
 * jeśli odpowiednio pierwszy tekst jest większy od drugiego lub drugi od pierwszego.
 */
static int text_sorter(const void *a, const void *b)
{
    wchar_t *A = *(wchar_t**)a;
    wchar_t *B = *(wchar_t**)b;
    
    while(*A == *B && *A != 0)
    {
        A++; B++;
    }
    return *A - *B;
}

/**
 * @}
 */

/**
 * @name Elementy interfejsu.
 * @{
 */

struct hint_rule * rule_make(const wchar_t *src, const wchar_t *dst, int cost, enum rule_flag flag)
{
    if(src == NULL || dst == NULL) return NULL;
    if(cost <= 0) return NULL;
    // e -> e must be split rule
    if(*src == 0 && *dst == 0 && flag != RULE_SPLIT) return NULL;
    // Check for too much variables in dst
    char memry[10];
    for(int i = 0; i < 10; i++)
        memry[i] = 0;
    const wchar_t *s = dst;
    while(*s != 0)
    {
        if(*s >= L'0' && *s <= L'9') memry[*s - L'0'] = 1;
        s++;
    }
    s = src;
    while(*s != 0)
    {
        if(*s >= L'0' && *s <= L'9') memry[*s - L'0'] = 0;
        s++;
    }
    int cnt = 0;
    for(int i = 0; i < 10; i++) cnt += memry[i];
    if(cnt > 1) return NULL;
    
    struct hint_rule *rule = malloc(sizeof(struct hint_rule));
    int sl = wcslen(src);
    int dl = wcslen(dst);
    rule->src = malloc((sl+1) * sizeof(wchar_t));
    memcpy(rule->src, src, (sl+1) * sizeof(wchar_t));
    rule->dst = malloc((dl+1) * sizeof(wchar_t));
    memcpy(rule->dst, dst, (dl+1) * sizeof(wchar_t));
    rule->cost = cost;
    rule->flag = flag;
    return rule;
}

void rule_done(struct hint_rule *rule)
{
    free(rule->src);
    free(rule->dst);
    free(rule);
}


struct list * rule_generate_hints(struct hint_rule **rules, int max_cost, int max_hints_no, struct trie_node *root, const wchar_t *word)
{
    int wlen = wcslen(word);
    struct list **pp = preprocess(rules, word);
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
            list_add_list_and_free(layers[i], apply_rules_to_states(layers[lno], j, root, pp, is));
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
                list_add(toadd, (void*)e);
            else
                free((void*)e);
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
    for(int i = 0; i <= max_cost; i++)
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

int rule_serialize(struct hint_rule *rule, FILE *file)
{
    struct string *src = string_make(rule->src);
    struct string *dst = string_make(rule->dst);
    if(string_serialize(src, file)<0) return -1;
    if(string_serialize(dst, file)<0) return -1;
    if(int32_serialize(rule->cost, file)<0) return -1;
    if(int32_serialize(rule->flag, file)<0) return -1;
    return 0;
}

struct hint_rule *rule_deserialize(FILE *file)
{
    struct string *src = string_deserialize(file);
    struct string *dst = string_deserialize(file);
    int cost;
    int flag;
    if(int32_deserialize(&cost, file)<0) goto fail;
    if(int32_deserialize(&flag, file)<0) goto fail;
    if(src == NULL || dst == NULL) goto fail;
    struct hint_rule *rule = malloc(sizeof(struct hint_rule));
    rule->src = string_undress(src);
    rule->dst = string_undress(dst);
    rule->cost = cost;
    rule->flag = flag;
    return rule;
fail:
    if(src != NULL) string_done(src);
    if(dst != NULL) string_done(dst);
    return NULL;
}

/**
 * @}
 */