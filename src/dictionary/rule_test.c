/** @file
  Test implementacji reguł podpowiedzi.
  
  @ingroup dictionary
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-06-15
 */

#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "rule.h"

struct hint_rule
{
    wchar_t *src;
    wchar_t *dst;
    int cost;
    enum rule_flag flag;
};

struct state
{
    const wchar_t *suf;
    const struct trie_node * node;
    const struct trie_node * prev;
    struct state *prnt;
    struct hint_rule *rule;
    wchar_t free_variable;
};

struct costed_state
{
    struct state *s;
    int cost;
};


extern bool pattern_matches(const wchar_t *pattern, const wchar_t *text, wchar_t memory[10]);
extern wchar_t translate_letter(wchar_t c, wchar_t memory[10]);
extern struct hint_rule *** preprocess_suffix(struct hint_rule **rules, int rcnt, const wchar_t *word, bool begin);
extern struct hint_rule **** preprocess(struct hint_rule **rules, const wchar_t *word);
extern void free_preprocessing_data_for_suffix(struct hint_rule ***pp);
extern void free_preprocessing_data(struct hint_rule ****pp, int wlen);
extern struct list * extend_state(struct state *s);
extern void explore_trie(const struct trie_node *n, wchar_t *dst, wchar_t memory[10], struct list *l, struct state *ps, struct hint_rule *r, const wchar_t *suf, const struct trie_node *root, wchar_t last_guessed);
extern struct list * apply_rule(struct state *s, struct hint_rule *r, const struct trie_node *root);
extern struct list * apply_rules_to_states(struct list *s, int c, const struct trie_node *root, struct hint_rule ****pp);
extern void unify_states(struct list **ll, int mc);
extern wchar_t * get_text(struct state *s);
extern int text_sorter(void *a, void *b);

/// Sprawdza dopasowanie wzorca bez zmiennych.
static void pattern_matches_no_vars_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10];
    assert_true(pattern_matches(L"okoń", L"okoń", memory));
    assert_false(pattern_matches(L"figa", L"makiem", memory));
    assert_true(pattern_matches(L"para", L"parapet", memory));
    assert_false(pattern_matches(L"paragraf", L"parapet", memory));
}

/// Sprawdza dopasowanie wzorca ze zmienną.
static void pattern_matches_jocker_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10];
    assert_true(pattern_matches(L"0ocker", L"jocker", memory));
    assert_true(pattern_matches(L"0ie1o23ik", L"niewolnik", memory));
    assert_true(pattern_matches(L"w4lo83", L"waloyo", memory));
    assert_true(pattern_matches(L"194ty", L"żółty", memory));
    assert_true(pattern_matches(L"p6aw3a", L"prawda", memory));
    assert_false(pattern_matches(L"p6aw3a", L"prawdę", memory));
}

/// Sprawdza dopasowanie wzorca z ustaloną zmienną.
static void pattern_matches_constrained_jocker_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10];
    assert_true(pattern_matches(L"0br0k0d0br0", L"abrakadabra", memory));
    assert_true(pattern_matches(L"1471578zi78e9", L"prapradziadek", memory));
    assert_true(pattern_matches(L"wsp0ni0ł0", L"wspaniała", memory));
    assert_false(pattern_matches(L"wsp0ni0ł0", L"wspaniały", memory));
}

/// Sprawdza tłumaczenie liter dla litery.
static void translate_letter_alpha_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10] = {L'i',L'j',L'k',L'l',L'm',L'n',L'o',L'p',L'q',L'r'};
    assert_int_equal(translate_letter(L'z', memory), L'z');
    assert_int_equal(translate_letter(L'ł', memory), L'ł');
    assert_int_equal(translate_letter(L'a', memory), L'a');
    assert_int_equal(translate_letter(L'ć', memory), L'ć');
    
}
/// Sprawdza tłumaczenie liter dla zmiennej.
static void translate_letter_jocker_1_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10] = {L'i',L'j',L'k',L'l',L'm',L'n',L'o',L'p',L'q',L'r'};
    assert_int_equal(translate_letter(L'0', memory), L'i');
    assert_int_equal(translate_letter(L'5', memory), L'n');
    assert_int_equal(translate_letter(L'7', memory), L'p');
    assert_int_equal(translate_letter(L'9', memory), L'r');
}
/// Sprawdza tłumaczenie liter dla zmiennej.
static void translate_letter_jocker_2_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10] = {L'a',L'ą',L'b',L'c',L'ć',L'd',L'e',L'ę',L'f',L'g'};
    assert_int_equal(translate_letter(L'0', memory), L'a');
    assert_int_equal(translate_letter(L'1', memory), L'ą');
    assert_int_equal(translate_letter(L'2', memory), L'b');
    assert_int_equal(translate_letter(L'3', memory), L'c');
    assert_int_equal(translate_letter(L'4', memory), L'ć');
    assert_int_equal(translate_letter(L'5', memory), L'd');
    assert_int_equal(translate_letter(L'6', memory), L'e');
    assert_int_equal(translate_letter(L'7', memory), L'ę');
    assert_int_equal(translate_letter(L'8', memory), L'f');
    assert_int_equal(translate_letter(L'9', memory), L'g');
}
/// Sprawdza tłumaczenie liter dla syfu.
static void translate_letter_junk_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10] = {L'i',L'j',L'k',L'l',L'm',L'n',L'o',L'p',L'q',L'r'};
    assert_int_equal(translate_letter(L'#', memory), -1);
    assert_int_equal(translate_letter(L'%', memory), -1);
    assert_int_equal(translate_letter(L'&', memory), -1);
    assert_int_equal(translate_letter(L'-', memory), -1);
}
/// Sprawdza tłumaczenie liter dla nieustalonej zmiennej.
static void translate_letter_unset_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10] = {0,0,0,0,0,0,0,0,0,0};
    assert_int_equal(translate_letter(L'0', memory), 0);
    assert_int_equal(translate_letter(L'1', memory), 1);
    assert_int_equal(translate_letter(L'2', memory), 2);
    assert_int_equal(translate_letter(L'3', memory), 3);
    assert_int_equal(translate_letter(L'4', memory), 4);
    assert_int_equal(translate_letter(L'5', memory), 5);
    assert_int_equal(translate_letter(L'6', memory), 6);
    assert_int_equal(translate_letter(L'7', memory), 7);
    assert_int_equal(translate_letter(L'8', memory), 8);
    assert_int_equal(translate_letter(L'9', memory), 9);
}
/// Testuje tworzenie i usuwanie reguł.
static void rule_make_done_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule *r = rule_make(L"pa773rn", L"d357ina7ion", 7, RULE_SPLIT);
    assert_string_equal(r->src, L"pa773rn");
    assert_string_equal(r->dst, L"d357ina7ion");
    assert_int_equal(r->cost, 7);
    assert_int_equal(r->flag, RULE_SPLIT);
    rule_done(r);
}
/// Testuje preprocessing (test bez reguł).
static void preprocess_suffix_no_rule_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule **rules = malloc(sizeof(struct hint_rule*));
    rules[0] = NULL;
    struct hint_rule ***output = preprocess_suffix(rules, 0, L"umlambo", false);
    assert_true(output[0] == NULL);
    free_preprocessing_data_for_suffix(output);
    free(rules);
}
/// Testuje preprocessing (test z niepasującą regułą).
static void preprocess_suffix_one_unmatching_rule_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule **rules = malloc(2 * sizeof(struct hint_rule*));
    rules[0] = rule_make(L"izolo", L"ngomso", 7, RULE_NORMAL);
    rules[1] = NULL;
    struct hint_rule ***output = preprocess_suffix(rules, 1, L"namhlanje", false);
    assert_true(output[0] == NULL);
    free_preprocessing_data_for_suffix(output);
    rule_done(rules[0]);
    free(rules);
}

/// Testuje preprocessing (test z pasującą regułą).
static void preprocess_suffix_one_matching_rule_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule **rules = malloc(2 * sizeof(struct hint_rule*));
    rules[0] = rule_make(L"izolo", L"ngomso", 7, RULE_NORMAL);
    rules[1] = NULL;
    struct hint_rule ***output = preprocess_suffix(rules, 1, L"izolo", false);
    assert_true(output[0] != NULL);
    assert_true(output[0][0] == NULL);
    assert_true(output[1][0] == NULL);
    assert_true(output[2][0] == NULL);
    assert_true(output[3][0] == NULL);
    assert_true(output[4][0] == NULL);
    assert_true(output[5][0] == NULL);
    assert_true(output[6][0] == NULL);
    assert_true(output[7][0] == rules[0]);
    assert_true(output[7][1] == NULL);
    assert_true(output[8] == NULL);
    free_preprocessing_data_for_suffix(output);
    rule_done(rules[0]);
    free(rules);
}
/// Testuje preprocessing (test z pasującymi regułami).
static void preprocess_suffix_some_samecost_rules_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule **rules = malloc(4 * sizeof(struct hint_rule*));
    rules[0] = rule_make(L"rz", L"ż", 1, RULE_NORMAL);
    rules[1] = rule_make(L"rzep", L"cokolwiek", 1, RULE_NORMAL);
    rules[2] = rule_make(L"0z", L"0ój", 1, RULE_NORMAL);
    rules[3] = NULL;
    struct hint_rule ***output = preprocess_suffix(rules, 3, L"rzepiasty", false);
    assert_true(output[0] != NULL);
    assert_true(output[0][0] == NULL);
    assert_true(output[1] != NULL);
    assert_true(output[1][0] == rules[0]
            ||  output[1][1] == rules[0]
            ||  output[1][2] == rules[0]);
    assert_true(output[1][0] == rules[1]
            ||  output[1][1] == rules[1]
            ||  output[1][2] == rules[1]);
    assert_true(output[1][0] == rules[2]
            ||  output[1][1] == rules[2]
            ||  output[1][2] == rules[2]);
    assert_true(output[1][3] == NULL);
    assert_true(output[2] == NULL);
    free_preprocessing_data_for_suffix(output);
    rule_done(rules[0]);
    rule_done(rules[1]);
    rule_done(rules[2]);
    free(rules);
}
/// Testuje preprocessing (test z pasującymi regułami).
static void preprocess_suffix_some_multicost_rules_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule **rules = malloc(5 * sizeof(struct hint_rule*));
    rules[0] = rule_make(L"rz", L"ż", 1, RULE_NORMAL);
    rules[1] = rule_make(L"rzep", L"cokolwiek", 1, RULE_NORMAL);
    rules[2] = rule_make(L"0z", L"0ój", 2, RULE_NORMAL);
    rules[3] = rule_make(L"01", L"10", 3, RULE_NORMAL);
    rules[4] = NULL;
    struct hint_rule ***output = preprocess_suffix(rules, 4, L"rzepiasty", false);
    assert_true(output[0] != NULL);
    assert_true(output[0][0] == NULL);
    assert_true(output[1] != NULL);
    assert_true(output[1][0] == rules[0]
            ||  output[1][1] == rules[0]);
    assert_true(output[1][0] == rules[1]
            ||  output[1][1] == rules[1]);
    assert_true(output[1][2] == NULL);
    assert_true(output[2] != NULL);
    assert_true(output[2][0] == rules[2]);
    assert_true(output[2][1] == NULL);
    assert_true(output[3] != NULL);
    assert_true(output[3][0] == rules[3]);
    assert_true(output[3][1] == NULL);
    assert_true(output[4] == NULL);
    
    free_preprocessing_data_for_suffix(output);
    rule_done(rules[0]);
    rule_done(rules[1]);
    rule_done(rules[2]);
    rule_done(rules[3]);
    free(rules);
}
/// Testuje preprocessing (test z regułą z flagą begin).
static void preprocess_suffix_begin_flag_1_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule **rules = malloc(2 * sizeof(struct hint_rule*));
    rules[0] = rule_make(L"nie", L"tak", 1, RULE_BEGIN);
    rules[1] = NULL;
    struct hint_rule ***output = preprocess_suffix(rules, 1, L"nieludzki", false);
    assert_true(output[0] == NULL);
    free_preprocessing_data_for_suffix(output);
    rule_done(rules[0]);
    free(rules);
}
/// Testuje preprocessing (test z regułą z flagą begin).
static void preprocess_suffix_begin_flag_2_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule **rules = malloc(2 * sizeof(struct hint_rule*));
    rules[0] = rule_make(L"nie", L"tak", 1, RULE_BEGIN);
    rules[1] = NULL;
    struct hint_rule ***output = preprocess_suffix(rules, 1, L"niewierzący", true);
    assert_true(output[0] != NULL);
    assert_true(output[0][0] == NULL);
    assert_true(output[1] != NULL);
    assert_true(output[1][0] == rules[0]);
    assert_true(output[1][1] == NULL);
    assert_true(output[2] == NULL);
    free_preprocessing_data_for_suffix(output);
    rule_done(rules[0]);
    free(rules);
}
/// Testuje preprocessing (test z regułą z flagą end).
static void preprocess_suffix_end_flag_1_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule **rules = malloc(2 * sizeof(struct hint_rule*));
    rules[0] = rule_make(L"nie", L"tak", 1, RULE_END);
    rules[1] = NULL;
    struct hint_rule ***output = preprocess_suffix(rules, 1, L"niekończący", false);
    assert_true(output[0] == NULL);
    free_preprocessing_data_for_suffix(output);
    rule_done(rules[0]);
    free(rules);
}
/// Testuje preprocessing (test z regułą z flagą end).
static void preprocess_suffix_end_flag_2_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct hint_rule **rules = malloc(2 * sizeof(struct hint_rule*));
    rules[0] = rule_make(L"nie", L"tak", 1, RULE_END);
    rules[1] = NULL;
    struct hint_rule ***output = preprocess_suffix(rules, 1, L"nie", false);
    assert_true(output[0] != NULL);
    assert_true(output[0][0] == NULL);
    assert_true(output[1] != NULL);
    assert_true(output[1][0] == rules[0]);
    assert_true(output[1][1] == NULL);
    assert_true(output[2] == NULL);
    free_preprocessing_data_for_suffix(output);
    rule_done(rules[0]);
    free(rules);
}
/// Testuje rozwijanie stanu.
static void extend_state_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"abcde");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    const struct trie_node *d2 = trie_get_child(d1, L'b');
    const struct trie_node *d3 = trie_get_child(d2, L'c');
    const struct trie_node *d4 = trie_get_child(d3, L'd');
    const struct trie_node *d5 = trie_get_child(d4, L'e');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"abcd";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    struct list *l = extend_state(s);
    struct state **ss = list_get(l);
    assert_int_equal(list_size(l), 5);
    assert_true(ss[0] == s);
    assert_true(ss[1]->suf == suf+1);
    assert_true(ss[1]->rule == NULL);
    assert_true(ss[1]->node == d1);
    assert_true(ss[1]->prev == NULL);
    assert_true(ss[1]->prnt == ss[0]);
    assert_true(ss[2]->suf == suf+2);
    assert_true(ss[2]->rule == NULL);
    assert_true(ss[2]->node == d2);
    assert_true(ss[2]->prev == NULL);
    assert_true(ss[2]->prnt == ss[1]);
    assert_true(ss[3]->suf == suf+3);
    assert_true(ss[3]->rule == NULL);
    assert_true(ss[3]->node == d3);
    assert_true(ss[3]->prev == NULL);
    assert_true(ss[3]->prnt == ss[2]);
    assert_true(ss[4]->suf == suf+4);
    assert_true(ss[4]->rule == NULL);
    assert_true(ss[4]->node == d4);
    assert_true(ss[4]->prev == NULL);
    assert_true(ss[4]->prnt == ss[3]);
    free(ss[0]);
    free(ss[1]);
    free(ss[2]);
    free(ss[3]);
    free(ss[4]);
    list_done(l);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę.
static void explore_trie_noway_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"a", 1, RULE_NORMAL);
    wchar_t memory[10];
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 0);
    
    free(s);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę.
static void explore_trie_letter_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"a");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"a", 1, RULE_NORMAL);
    wchar_t memory[10];
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 1);
    struct state **ss = list_get(l);
    assert_true(ss[0]->node == d1);
    assert_true(ss[0]->prev == NULL);
    assert_true(ss[0]->prnt == s);
    assert_true(ss[0]->rule == r);
    assert_int_equal(ss[0]->suf, suf);
    
    free(s);
    free(ss[0]);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę.
static void explore_trie_constrained_jocker_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"a");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"7", 1, RULE_NORMAL);
    wchar_t memory[10];
    memory[7] = L'a';
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 1);
    struct state **ss = list_get(l);
    assert_true(ss[0]->node == d1);
    assert_true(ss[0]->prev == NULL);
    assert_true(ss[0]->prnt == s);
    assert_true(ss[0]->rule == r);
    assert_int_equal(ss[0]->suf, suf);
    
    free(s);
    free(ss[0]);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę (test z wolną zmienną).
static void explore_trie_oneway_jocker_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"a");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"7", 1, RULE_NORMAL);
    wchar_t memory[10];
    memory[7] = 0;
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 1);
    struct state **ss = list_get(l);
    assert_true(ss[0]->node == d1);
    assert_true(ss[0]->prev == NULL);
    assert_true(ss[0]->prnt == s);
    assert_true(ss[0]->rule == r);
    assert_int_equal(ss[0]->suf, suf);
    
    free(s);
    free(ss[0]);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę (test z wolną zmienną).
static void explore_trie_multiway_jocker_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"a");
    trie_insert(d, L"z");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    const struct trie_node *d2 = trie_get_child(d, L'z');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"7", 1, RULE_NORMAL);
    wchar_t memory[10];
    memory[7] = 0;
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 2);
    struct state **ss = list_get(l);
    assert_true(ss[0]->node == d1);
    assert_true(ss[0]->prev == NULL);
    assert_true(ss[0]->prnt == s);
    assert_true(ss[0]->rule == r);
    assert_int_equal(ss[0]->suf, suf);
    assert_int_equal(ss[0]->free_variable, L'a');
    
    assert_true(ss[1]->node == d2);
    assert_true(ss[1]->prev == NULL);
    assert_true(ss[1]->prnt == s);
    assert_true(ss[1]->rule == r);
    assert_int_equal(ss[1]->suf, suf);
    assert_int_equal(ss[1]->free_variable, L'z');
    
    free(s);
    free(ss[0]);
    free(ss[1]);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę (test z wolną zmienną).
static void explore_trie_self_constraint_jocker_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"aa");
    trie_insert(d, L"az");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    const struct trie_node *d2 = trie_get_child(d1, L'a');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"77", 1, RULE_NORMAL);
    wchar_t memory[10];
    memory[7] = 0;
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 1);
    struct state **ss = list_get(l);
    assert_true(ss[0]->node == d2);
    assert_true(ss[0]->prev == NULL);
    assert_true(ss[0]->prnt == s);
    assert_true(ss[0]->rule == r);
    assert_int_equal(ss[0]->suf, suf);
    
    free(s);
    free(ss[0]);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę (test z regułą kończącą).
static void explore_trie_end_rule_1_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"ac");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"a", 1, RULE_END);
    wchar_t memory[10];
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 0);
    
    free(s);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę (test z regułą kończącą).
static void explore_trie_end_rule_2_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"ac");
    trie_insert(d, L"a");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"a", 1, RULE_END);
    wchar_t memory[10];
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 1);
    struct state **ss = list_get(l);
    assert_true(ss[0]->node == d1);
    assert_true(ss[0]->prev == NULL);
    assert_true(ss[0]->prnt == s);
    assert_true(ss[0]->rule == r);
    assert_int_equal(ss[0]->suf, suf);
    
    free(s);
    free(ss[0]);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę (test z regułą dzielącą).
static void explore_trie_split_rule_1_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"ac");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"a", 1, RULE_SPLIT);
    wchar_t memory[10];
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 0);
    
    free(s);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję pomocniczą aplikującą regułę (test z regułą dzielącą).
static void explore_trie_split_rule_2_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"ac");
    trie_insert(d, L"a");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"b";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"b", L"a", 1, RULE_SPLIT);
    wchar_t memory[10];
    
    struct list *l = list_init();
    
    explore_trie(d, r->dst, memory, l, s, r, suf, d, 0);
    
    assert_int_equal(list_size(l), 1);
    struct state **ss = list_get(l);
    assert_true(ss[0]->node == d);
    assert_true(ss[0]->prev == d1);
    assert_true(ss[0]->prnt == s);
    assert_true(ss[0]->rule == r);
    assert_int_equal(ss[0]->suf, suf);
    
    free(s);
    free(ss[0]);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję aplikującą regułę do stanu.
static void apply_rule_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"aa");
    trie_insert(d, L"az");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    const struct trie_node *d2 = trie_get_child(d1, L'z');
    
    struct state *s = malloc(sizeof(struct state));
    const wchar_t *suf = L"za";
    s->node = d;
    s->prev = NULL;
    s->prnt = NULL;
    s->rule = NULL;
    s->suf = suf;
    
    struct hint_rule *r = rule_make(L"01", L"10", 1, RULE_NORMAL);
    wchar_t memory[10] = {0,0,0,0,0,0,0,0,0,0};
    
    struct list *l = apply_rule(s, r, d);
    
    assert_int_equal(list_size(l), 1);
    struct state **ss = list_get(l);
    assert_true(ss[0]->node == d2);
    assert_true(ss[0]->prev == NULL);
    assert_true(ss[0]->prnt == s);
    assert_true(ss[0]->rule == r);
    assert_int_equal(ss[0]->suf, suf + 2);
    
    free(s);
    free(ss[0]);
    list_done(l);
    rule_done(r);
    trie_done(d);
}
/// Testuje funkcję aplikującą reguły do stanów.
static void apply_rules_to_states_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"aa");
    trie_insert(d, L"az");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    const struct trie_node *d2 = trie_get_child(d1, L'z');
    
    struct list * states = list_init();
    struct state *s1 = malloc(sizeof(struct state));
    list_add(states, s1);
    const wchar_t *suf = L"za";
    s1->node = d;
    s1->prev = NULL;
    s1->prnt = NULL;
    s1->rule = NULL;
    s1->suf = suf;
    
    struct hint_rule * rules[3];
    struct hint_rule *r1 = rules[0] = rule_make(L"01", L"10", 1, RULE_NORMAL);
    struct hint_rule *r2 = rules[1] = rule_make(L"z", L"", 1, RULE_BEGIN);
    rules[2] = NULL;
    
    struct hint_rule ****pp = preprocess(rules, suf);
    struct list * l = apply_rules_to_states(states, 1, d, pp);
    
    assert_int_equal(list_size(l), 3);
    struct state **ss = list_get(l);
    struct state *sa = NULL;
    struct state *sb = NULL;
    struct state *sc = NULL;

    for(int i = 0; i < 3; i++)
        if(ss[i]->node == d) sa = ss[i];
        
    for(int i = 0; i < 3; i++)
        if(ss[i]->node == d2) sb = ss[i];
        
    for(int i = 0; i < 3; i++)
        if(ss[i]->node == d1) sc = ss[i];
    
    assert_true(sa != NULL);
    assert_true(sb != NULL);
    assert_true(sc != NULL);
        
    assert_true(sa->node == d);
    assert_true(sa->prev == NULL);
    assert_true(sa->prnt == s1);
    assert_true(sa->rule == r2);
    assert_true(sa->suf == suf + 1);
    
    assert_true(sb->node == d2);
    assert_true(sb->prev == NULL);
    assert_true(sb->prnt == s1);
    assert_true(sb->rule == r1);
    assert_true(sb->suf == suf + 2);
    
    assert_true(sc->node == d1);
    assert_true(sc->prev == NULL);
    assert_true(sc->prnt == sa);
    assert_true(sc->rule == NULL);
    assert_true(sc->suf == suf + 2);
    
    free(sa);
    free(sb);
    free(sc);
    list_done(l);
    free_preprocessing_data(pp, 2);
    
    rule_done(r1);
    rule_done(r2);
    
    free(s1);
    list_done(states);
    trie_done(d);
}

/// Testuje funkcję aplikującą reguły do stanów.
static void apply_rules_to_states_closed_state_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"aa");
    trie_insert(d, L"az");
    trie_insert(d, L"azy");
    const struct trie_node *d1 = trie_get_child(d, L'a');
    const struct trie_node *d2 = trie_get_child(d1, L'z');
 
    struct hint_rule *cr = rule_make(L"za", L"az", 1, RULE_END);
    
    struct list * states = list_init();
    struct state *s1 = malloc(sizeof(struct state));
    list_add(states, s1);
    const wchar_t *suf = L"za";
    s1->node = d;
    s1->prev = NULL;
    s1->prnt = NULL;
    s1->rule = cr;
    s1->suf = suf;
    
    struct hint_rule * rules[3];
    struct hint_rule *r1 = rules[0] = rule_make(L"", L"y", 1, RULE_NORMAL);
    rules[1] = cr;
    rules[2] = NULL;
    
    struct hint_rule ****pp = preprocess(rules, suf);
    struct list * l = apply_rules_to_states(states, 1, d, pp);
    
    assert_int_equal(list_size(l), 0);
    
    list_done(l);
    free_preprocessing_data(pp, 2);
    
    rule_done(r1);
    rule_done(cr);
    
    free(s1);
    list_done(states);
    trie_done(d);
}

/**
 * Tworzy stan.
 * @param[in] n Aktualny węzeł drzewa.
 * @param[in] m Węzeł reprezentujący poprzednie słowo.
 * @param[in] p Poprzedni stan.
 * @param[in] r Użyta reguła.
 * @param[in] suf Sufiks.
 * @return Wskaźnik na stan.
 */
static struct state * mkstate(const struct trie_node *n, const struct trie_node *m, struct state *p, struct hint_rule *r, const wchar_t *suf)
{
    struct state *s = malloc(sizeof(struct state));
    s->node = n;
    s->prev = m;
    s->prnt = p;
    s->rule = r;
    s->suf = suf;
    return s;
}

/// Testuje usuwanie duplikatów stanów.
static void unify_states_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"c");
    trie_insert(d, L"cd");
    trie_insert(d, L"d");
    struct trie_node *d1 = trie_get_child(d, L'c');
    struct trie_node *d2 = trie_get_child(d1, L'd');
    struct trie_node *d3 = trie_get_child(d, L'd');
    
    struct hint_rule *r[5];
    r[0] = rule_make(L"ab", L"c", 1, RULE_END);
    r[1] = rule_make(L"a", L"c", 1, RULE_NORMAL);
    r[2] = rule_make(L"b", L"", 1, RULE_NORMAL);
    r[3] = rule_make(L"", L"d", 1, RULE_NORMAL);
    r[4] = rule_make(L"", L"", 1, RULE_SPLIT);
    
    struct list *l[5];
    l[0] = list_init();
    l[1] = list_init();
    l[2] = list_init();
    l[3] = list_init();
    l[4] = list_init();
    
    const wchar_t *word = L"ab";
    
    struct state *s00 = mkstate(d, NULL, NULL, NULL, word);
    struct state *s10 = mkstate(d1,NULL, s00, r[0], word + 2);
    struct state *s11 = mkstate(d1,NULL, s00, r[1], word + 1);
    struct state *s20 = mkstate(d1,NULL, s11, r[2], word + 2);
    struct state *s21 = mkstate(d, d1,   s11, r[4], word + 1);
    struct state *s30 = mkstate(d2,NULL, s20, r[3], word + 2);
    struct state *s31 = mkstate(d, d1,   s21, r[2], word + 2);
    struct state *s32 = mkstate(d1,NULL, s11, r[4], word + 2);
    struct state *s40 = mkstate(d3,d1,   s31, r[3], word + 2);
    list_add(l[0], s00);
    list_add(l[1], s10); list_add(l[1], s11);
    list_add(l[2], s20); list_add(l[2], s21);
    list_add(l[3], s30); list_add(l[3], s31); list_add(l[3], s32);
    list_add(l[4], s40);
    
    unify_states(l, 4);
    assert_int_equal(list_size(l[0]), 1);
    assert_int_equal(list_size(l[1]), 2);
    assert_int_equal(list_size(l[2]), 2);
    assert_int_equal(list_size(l[3]), 2);
    assert_int_equal(list_size(l[4]), 1);
    assert_true(list_get(l[0])[0] == s00);
    assert_true(list_get(l[1])[0] == s10 || list_get(l[1])[1] == s10);
    assert_true(list_get(l[1])[0] == s11 || list_get(l[1])[1] == s11);
    assert_true(list_get(l[2])[0] == s20 || list_get(l[2])[1] == s20);
    assert_true(list_get(l[2])[0] == s21 || list_get(l[2])[1] == s21);
    assert_true(list_get(l[3])[0] == s30 || list_get(l[3])[1] == s30);
    assert_true(list_get(l[3])[0] == s31 || list_get(l[3])[1] == s31);
    assert_true(list_get(l[4])[0] == s40);
    
    free(s00);
    free(s10);
    free(s11);
    free(s20);
    free(s21);
    free(s30);
    free(s31);
    // State s32 was removed by unification
    free(s40);
    
    list_done(l[0]);
    list_done(l[1]);
    list_done(l[2]);
    list_done(l[3]);
    list_done(l[4]);
    
    rule_done(r[0]);
    rule_done(r[1]);
    rule_done(r[2]);
    rule_done(r[3]);
    rule_done(r[4]);
    
    trie_done(d);
}

/// Testuje zamianę stanu na tekst.
static void get_text_test(void **rubbish)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"c");
    trie_insert(d, L"cd");
    trie_insert(d, L"d");
    struct trie_node *d1 = trie_get_child(d, L'c');
    struct trie_node *d2 = trie_get_child(d1, L'd');
    struct trie_node *d3 = trie_get_child(d, L'd');
    
    struct hint_rule *r[5];
    r[0] = rule_make(L"ab", L"c", 1, RULE_END);
    r[1] = rule_make(L"a", L"c", 1, RULE_NORMAL);
    r[2] = rule_make(L"b", L"", 1, RULE_NORMAL);
    r[3] = rule_make(L"", L"d", 1, RULE_NORMAL);
    r[4] = rule_make(L"", L"", 1, RULE_SPLIT);
    
    struct list **l[5];
    l[0] = list_init();
    l[1] = list_init();
    l[2] = list_init();
    l[3] = list_init();
    l[4] = list_init();
    
    const wchar_t *word = L"ab";
    
    struct state *s00 = mkstate(d, NULL, NULL, NULL, word);
    struct state *s10 = mkstate(d1,NULL, s00, r[0], word + 2);
    struct state *s11 = mkstate(d1,NULL, s00, r[1], word + 1);
    struct state *s20 = mkstate(d1,NULL, s11, r[2], word + 2);
    struct state *s21 = mkstate(d, d1,   s11, r[4], word + 1);
    struct state *s30 = mkstate(d2,NULL, s20, r[3], word + 2);
    struct state *s31 = mkstate(d, d1,   s21, r[2], word + 2);
    struct state *s40 = mkstate(d3,d1,   s31, r[3], word + 2);
    list_add(l[0], s00);
    list_add(l[1], s10); list_add(l[1], s11);
    list_add(l[2], s20); list_add(l[2], s21);
    list_add(l[3], s30); list_add(l[3], s31);
    list_add(l[4], s40);
    
    wchar_t *s;
    
    s = get_text(s00);
    assert_true(wcscmp(s, L"")==0);
    free(s);
    
    s = get_text(s10);
    assert_true(wcscmp(s, L"c")==0);
    free(s);
    
    s = get_text(s11);
    assert_true(wcscmp(s, L"c")==0);
    free(s);
    
    s = get_text(s20);
    assert_true(wcscmp(s, L"c")==0);
    free(s);
    
    s = get_text(s21);
    assert_true(wcscmp(s, L"c ")==0);
    free(s);
    
    s = get_text(s30);
    assert_true(wcscmp(s, L"cd")==0);
    free(s);
    
    s = get_text(s31);
    assert_true(wcscmp(s, L"c ")==0);
    free(s);
    
    s = get_text(s40);
    assert_true(wcscmp(s, L"c d")==0);
    free(s);
    
    free(s00);
    free(s10);
    free(s11);
    free(s20);
    free(s21);
    free(s30);
    free(s31);
    // State s32 was removed by unification
    free(s40);
    
    list_done(l[0]);
    list_done(l[1]);
    list_done(l[2]);
    list_done(l[3]);
    list_done(l[4]);
    
    rule_done(r[0]);
    rule_done(r[1]);
    rule_done(r[2]);
    rule_done(r[3]);
    rule_done(r[4]);
    
    trie_done(d);
}

/// Testuje funkcję porównującą teksty.
static void text_sorter_test(void **state)
{
    const wchar_t *A = L"c";
    const wchar_t *B = L"cd";
    assert_true(text_sorter(&A, &B) != 0);
}

/// Testuje generowanie podpowiedzi.
static void rule_generate_hints_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    struct trie_node *d = trie_init();
    trie_insert(d, L"c");
    trie_insert(d, L"cd");
    trie_insert(d, L"cdd");
    trie_insert(d, L"dd");
    struct trie_node *d1 = trie_get_child(d, L'c');
    struct trie_node *d2 = trie_get_child(d1, L'd');
    struct trie_node *d3 = trie_get_child(d, L'd');
    
    struct hint_rule *r[6];
    r[0] = rule_make(L"ab", L"c", 1, RULE_END);
    r[1] = rule_make(L"a", L"c", 1, RULE_NORMAL);
    r[2] = rule_make(L"b", L"", 1, RULE_NORMAL);
    r[3] = rule_make(L"", L"d", 1, RULE_NORMAL);
    r[4] = rule_make(L"", L"", 1, RULE_SPLIT);
    r[5] = NULL;
    
    struct list *l = rule_generate_hints(r, 10, 100, d, L"ab");
    assert_int_equal(list_size(l), 9);
    wchar_t **ss = (wchar_t **)list_get(l);
    assert_true(wcscmp(ss[0], L"c")==0);
    assert_true(wcscmp(ss[1], L"cd")==0);
    assert_true(wcscmp(ss[2], L"cdd")==0);
    assert_true(wcscmp(ss[3], L"dd c")==0);
    assert_true(wcscmp(ss[4], L"c dd")==0);
    assert_true(wcscmp(ss[5], L"cd dd")==0);
    assert_true(wcscmp(ss[6], L"dd cd")==0);
    assert_true(wcscmp(ss[7], L"cdd dd")==0);
    assert_true(wcscmp(ss[8], L"dd cdd")==0);
    for(int i = 0; i < list_size(l); i++)
    {
        free(ss[i]);
    }
    list_done(l);
    rule_done(r[0]);
    rule_done(r[1]);
    rule_done(r[2]);
    rule_done(r[3]);
    rule_done(r[4]);
    
    trie_done(d);
}

/// Uruchamia testy.
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(pattern_matches_no_vars_test),
        cmocka_unit_test(pattern_matches_jocker_test),
        cmocka_unit_test(pattern_matches_constrained_jocker_test),
        cmocka_unit_test(translate_letter_alpha_test),
        cmocka_unit_test(translate_letter_jocker_1_test),
        cmocka_unit_test(translate_letter_jocker_2_test),
        cmocka_unit_test(translate_letter_junk_test),
        cmocka_unit_test(translate_letter_unset_test),
        cmocka_unit_test(rule_make_done_test),
        cmocka_unit_test(preprocess_suffix_no_rule_test),
        cmocka_unit_test(preprocess_suffix_one_unmatching_rule_test),
        cmocka_unit_test(preprocess_suffix_one_matching_rule_test),
        cmocka_unit_test(preprocess_suffix_some_samecost_rules_test),
        cmocka_unit_test(preprocess_suffix_some_multicost_rules_test),
        cmocka_unit_test(preprocess_suffix_begin_flag_1_test),
        cmocka_unit_test(preprocess_suffix_begin_flag_2_test),
        cmocka_unit_test(preprocess_suffix_end_flag_1_test),
        cmocka_unit_test(preprocess_suffix_end_flag_2_test),
        cmocka_unit_test(extend_state_test),
        cmocka_unit_test(explore_trie_noway_test),
        cmocka_unit_test(explore_trie_letter_test),
        cmocka_unit_test(explore_trie_constrained_jocker_test),
        cmocka_unit_test(explore_trie_oneway_jocker_test),
        cmocka_unit_test(explore_trie_multiway_jocker_test),
        cmocka_unit_test(explore_trie_self_constraint_jocker_test),
        cmocka_unit_test(explore_trie_end_rule_1_test),
        cmocka_unit_test(explore_trie_end_rule_2_test),
        cmocka_unit_test(explore_trie_split_rule_1_test),
        cmocka_unit_test(explore_trie_split_rule_2_test),
        cmocka_unit_test(apply_rule_test),
        cmocka_unit_test(apply_rules_to_states_test),
        cmocka_unit_test(apply_rules_to_states_closed_state_test),
        cmocka_unit_test(unify_states_test),
        cmocka_unit_test(get_text_test),
        cmocka_unit_test(text_sorter_test),
        cmocka_unit_test(rule_generate_hints_test),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
