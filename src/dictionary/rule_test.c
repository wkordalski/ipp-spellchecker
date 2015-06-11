/** @file
  Test implementacji reguł podpowiedzi.
  
  @ingroup dictionary
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-05-31
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


extern bool pattern_matches(const wchar_t *pattern, const wchar_t *text, wchar_t memory[10]);
extern wchar_t translate_letter(wchar_t c, wchar_t memory[10]);

static void pattern_matches_no_vars_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10];
    assert_true(pattern_matches(L"okoń", L"okoń", memory));
    assert_false(pattern_matches(L"figa", L"makiem", memory));
    assert_true(pattern_matches(L"para", L"parapet", memory));
    assert_false(pattern_matches(L"paragraf", L"parapet", memory));
}

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

static void pattern_matches_constrained_jocker_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10];
    assert_true(pattern_matches(L"0br0k0d0br0", L"abrakadabra", memory));
    assert_true(pattern_matches(L"1471578zi78e9", L"prapradziadek", memory));
    assert_true(pattern_matches(L"wsp0ni0ł0", L"wspaniała", memory));
    assert_false(pattern_matches(L"wsp0ni0ł0", L"wspaniały", memory));
}

static void translate_letter_alpha_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10] = {L'i',L'j',L'k',L'l',L'm',L'n',L'o',L'p',L'q',L'r'};
    assert_int_equal(translate_letter(L'z', memory), L'z');
    assert_int_equal(translate_letter(L'ł', memory), L'ł');
    assert_int_equal(translate_letter(L'a', memory), L'a');
    assert_int_equal(translate_letter(L'ć', memory), L'ć');
    
}
static void translate_letter_jocker_1_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10] = {L'i',L'j',L'k',L'l',L'm',L'n',L'o',L'p',L'q',L'r'};
    assert_int_equal(translate_letter(L'0', memory), L'i');
    assert_int_equal(translate_letter(L'5', memory), L'n');
    assert_int_equal(translate_letter(L'7', memory), L'p');
    assert_int_equal(translate_letter(L'9', memory), L'r');
}
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
static void translate_letter_junk_test(void **state)
{
    setlocale(LC_ALL, "pl_PL.UTF8");
    wchar_t memory[10] = {L'i',L'j',L'k',L'l',L'm',L'n',L'o',L'p',L'q',L'r'};
    assert_int_equal(translate_letter(L'#', memory), -1);
    assert_int_equal(translate_letter(L'%', memory), -1);
    assert_int_equal(translate_letter(L'&', memory), -1);
    assert_int_equal(translate_letter(L'-', memory), -1);
}

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

static void rule_make_done_test(void **state)
{
    struct hint_rule *r = rule_make(L"pa773rn", L"d3571na710n", 7, RULE_SPLIT);
    assert_string_equal(r->src, L"pa773rn");
    assert_string_equal(r->dst, L"d3571na710n");
    assert_int_equal(r->cost, 7);
    assert_int_equal(r->flag, RULE_SPLIT);
    rule_done(r);
}

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
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
