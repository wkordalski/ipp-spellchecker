/** @file
  Test implementacji drzewa TRIE.
  
  @ingroup dictionary
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-06-03
 */

#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "trie.h"
#include "word_list.h"
#include "../testable.h"

/**
 * Reprezentuje węzeł drzewa TRIE.
 */
struct trie_node
{
    struct trie_node **chd;     ///< Lista dzieci
    wchar_t val;                ///< Wartość węzła
    unsigned int cap;           ///< Pojemność tablicy dzieci
    unsigned int cnt;           ///< Ilość dzieci
    unsigned int leaf;          ///< Czy tutaj kończy się słowo
};

extern int trie_get_child_index(struct trie_node *node, wchar_t value, int begin, int end);
extern struct trie_node * trie_get_child_priv(struct trie_node *node, wchar_t value);
extern struct trie_node * trie_get_child_or_add_empty(struct trie_node *node, wchar_t value);
extern void trie_cleanup(struct trie_node *node, struct trie_node *parent);
extern int trie_delete_helper(struct trie_node *node, struct trie_node *parent, const wchar_t *word);
extern int trie_serialize_formatU_helper(struct trie_node *node, FILE *file);
extern int trie_serialize_formatU(struct trie_node *node, FILE *file);
extern int trie_deserialize_formatU_helper(struct trie_node *node, FILE *file);
extern struct trie_node * trie_deserialize_formatU(FILE *file);
extern void trie_hints_helper(struct trie_node *node, const wchar_t *word,
                       wchar_t **created, int length, int *capacity,
                       int points, struct word_list *list);

/**
 * Testuje tworzenie i usuwanie drzewa.
 */
static void trie_init_done_test(void **state)
{
    struct trie_node *node = trie_init();
    assert_true(node != NULL);
    assert_true(node->cnt == 0);
    assert_true(node->leaf == 0);
    trie_done(node);
}

/**
 * Tworzy środowisko z jednym węzłem.
 */
static int node_0_setup(void **state)
{
    *state = trie_init();
    return 0;
}

/**
 * Tworzy środowisko z węzłem mającym jedno dziecko.
 */
static int node_1_setup(void **state)
{
    struct trie_node *node = trie_init();
    node->cap = 4;
    node->cnt = 1;
    node->chd = malloc(sizeof(struct trie_node*)*4);
    *state = node;
    
    for(int i = 0; i < 1; i++)
    {
        struct trie_node *child = malloc(sizeof(struct trie_node));
        child->cap = 0;
        child->cnt = 0;
        child->chd = NULL;
        node->chd[i] = child;
    }
    node->chd[0]->val = 'c';
    return 0;
}

/**
 * Tworzy środowisko z węzłem mającym dwoje dzieci.
 */
static int node_2_setup(void **state)
{
    struct trie_node *node = trie_init();
    node->cap = 4;
    node->cnt = 2;
    node->chd = malloc(sizeof(struct trie_node*)*4);
    *state = node;
    
    for(int i = 0; i < 2; i++)
    {
        struct trie_node *child = malloc(sizeof(struct trie_node));
        child->cap = 0;
        child->cnt = 0;
        child->chd = NULL;
        node->chd[i] = child;
    }
    node->chd[0]->val = 'd';
    node->chd[1]->val = 's';
    return 0;
}

/**
 * Tworzy środowisko z węzłem mającym czwórkę dzieci.
 */
static int node_4_setup(void **state)
{
    struct trie_node *node = trie_init();
    node->cap = 4;
    node->cnt = 4;
    node->chd = malloc(sizeof(struct trie_node*)*4);
    *state = node;
    
    for(int i = 0; i < 4; i++)
    {
        struct trie_node *child = malloc(sizeof(struct trie_node));
        child->cap = 0;
        child->cnt = 0;
        child->chd = NULL;
        node->chd[i] = child;
    }
    node->chd[0]->val = 'd';
    node->chd[1]->val = 'f';
    node->chd[2]->val = 'h';
    node->chd[3]->val = 's';
    return 0;
}

/**
 * Usuwa środowisko z jednym węzłem.
 */
static int node_0_teardown(void **state)
{
    struct trie_node *node = *state;
    assert_true(node->chd == NULL);
    assert_true(node->cnt == 0);
    free(node);
    return 0;
}

/**
 * Usuwa środowisko z węzłem mającym jedno dziecko.
 */
static int node_1_teardown(void **state)
{
    struct trie_node *node = *state;
    assert_true(node->chd != NULL);
    assert_true(node->cnt == 1);
    for(int i = 0; i < 1; i++)
    {
        free(node->chd[i]);
    }
    free(node->chd);
    free(node);
    return 0;
}

/**
 * Usuwa środowisko z węzłem mającym dwójkę dzieci.
 */
static int node_2_teardown(void **state)
{
    struct trie_node *node = *state;
    assert_true(node->chd != NULL);
    assert_true(node->cnt == 2);
    for(int i = 0; i < 2; i++)
    {
        free(node->chd[i]);
    }
    free(node->chd);
    free(node);
    return 0;
}

/**
 * Usuwa środowisko z węzłem mającym czwórkę dzieci.
 */
static int node_4_teardown(void **state)
{
    struct trie_node *node = *state;
    assert_true(node->chd != NULL);
    assert_true(node->cnt == 4);
    for(int i = 0; i < 4; i++)
    {
        free(node->chd[i]);
    }
    free(node->chd);
    free(node);
    return 0;
}

/**
 * Testuje pobranie indeksu, gdzie powinien się znaleźć syn
 * o danej etykiecie na pojedynczym węźle.
 */
static void trie_get_child_index_empty_test(void **state)
{
    struct trie_node *node = *state;
    assert_int_equal(trie_get_child_index(node, L'a',0,0), -1);
}

/**
 * Testuje pobranie indeksu, gdzie powinien się znaleźć syn
 * o danej etykiecie na węźle z jednym dzieckiem.
 */
static void trie_get_child_index_1_test(void **state)
{
    struct trie_node *node = *state;
    assert_int_equal(trie_get_child_index(node, L'c',0,1), 0);
}

/**
 * Testuje pobranie indeksu, gdzie powinien się znaleźć syn
 * o danej etykiecie na węźle z dwoma dziećmi.
 */
static void trie_get_child_index_2_test(void **state)
{
    struct trie_node *node = *state;
    assert_int_equal(trie_get_child_index(node, L'a',0,1), 0);
    assert_int_equal(trie_get_child_index(node, L'a',1,2), 1);
    assert_int_equal(trie_get_child_index(node, L'a',0,2), 0);
    assert_int_equal(trie_get_child_index(node, L'd',0,1), 0);
    assert_int_equal(trie_get_child_index(node, L'd',1,2), 1);
    assert_int_equal(trie_get_child_index(node, L'd',0,2), 0);
    assert_int_equal(trie_get_child_index(node, L'o',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L'o',1,2), 1);
    assert_int_equal(trie_get_child_index(node, L'o',0,2), 1);
    assert_int_equal(trie_get_child_index(node, L's',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L's',1,2), 1);
    assert_int_equal(trie_get_child_index(node, L's',0,2), 1);
    assert_int_equal(trie_get_child_index(node, L'z',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L'z',1,2), 2);
    assert_int_equal(trie_get_child_index(node, L'z',0,2), 2);
}

/**
 * Testuje pobranie indeksu, gdzie powinien się znaleźć syn
 * o danej etykiecie na węźle z czterema dziećmi.
 */
static void trie_get_child_index_4_test(void **state)
{
    struct trie_node *node = *state;
    assert_int_equal(trie_get_child_index(node, L'a',0,0), 0);
    assert_int_equal(trie_get_child_index(node, L'a',0,1), 0);
    assert_int_equal(trie_get_child_index(node, L'a',0,2), 0);
    assert_int_equal(trie_get_child_index(node, L'a',0,3), 0);
    assert_int_equal(trie_get_child_index(node, L'a',0,4), 0);
    assert_int_equal(trie_get_child_index(node, L'a',1,1), 1);
    assert_int_equal(trie_get_child_index(node, L'a',1,2), 1);
    assert_int_equal(trie_get_child_index(node, L'a',1,3), 1);
    assert_int_equal(trie_get_child_index(node, L'a',1,4), 1);
    assert_int_equal(trie_get_child_index(node, L'a',2,2), 2);
    assert_int_equal(trie_get_child_index(node, L'a',2,3), 2);
    assert_int_equal(trie_get_child_index(node, L'a',2,4), 2);
    assert_int_equal(trie_get_child_index(node, L'a',3,3), 3);
    assert_int_equal(trie_get_child_index(node, L'a',3,4), 3);
    assert_int_equal(trie_get_child_index(node, L'a',4,4), 4);
    assert_int_equal(trie_get_child_index(node, L'd',0,0), 0);
    assert_int_equal(trie_get_child_index(node, L'd',0,1), 0);
    assert_int_equal(trie_get_child_index(node, L'd',0,2), 0);
    assert_int_equal(trie_get_child_index(node, L'd',0,3), 0);
    assert_int_equal(trie_get_child_index(node, L'd',0,4), 0);
    assert_int_equal(trie_get_child_index(node, L'd',1,1), 1);
    assert_int_equal(trie_get_child_index(node, L'd',1,2), 1);
    assert_int_equal(trie_get_child_index(node, L'd',1,3), 1);
    assert_int_equal(trie_get_child_index(node, L'd',1,4), 1);
    assert_int_equal(trie_get_child_index(node, L'd',2,2), 2);
    assert_int_equal(trie_get_child_index(node, L'd',2,3), 2);
    assert_int_equal(trie_get_child_index(node, L'd',2,4), 2);
    assert_int_equal(trie_get_child_index(node, L'd',3,3), 3);
    assert_int_equal(trie_get_child_index(node, L'd',3,4), 3);
    assert_int_equal(trie_get_child_index(node, L'd',4,4), 4);
    assert_int_equal(trie_get_child_index(node, L'e',0,0), 0);
    assert_int_equal(trie_get_child_index(node, L'e',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L'e',0,2), 1);
    assert_int_equal(trie_get_child_index(node, L'e',0,3), 1);
    assert_int_equal(trie_get_child_index(node, L'e',0,4), 1);
    assert_int_equal(trie_get_child_index(node, L'e',1,1), 1);
    assert_int_equal(trie_get_child_index(node, L'e',1,2), 1);
    assert_int_equal(trie_get_child_index(node, L'e',1,3), 1);
    assert_int_equal(trie_get_child_index(node, L'e',1,4), 1);
    assert_int_equal(trie_get_child_index(node, L'e',2,2), 2);
    assert_int_equal(trie_get_child_index(node, L'e',2,3), 2);
    assert_int_equal(trie_get_child_index(node, L'e',2,4), 2);
    assert_int_equal(trie_get_child_index(node, L'e',3,3), 3);
    assert_int_equal(trie_get_child_index(node, L'e',3,4), 3);
    assert_int_equal(trie_get_child_index(node, L'e',4,4), 4);
    assert_int_equal(trie_get_child_index(node, L'f',0,0), 0);
    assert_int_equal(trie_get_child_index(node, L'f',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L'f',0,2), 1);
    assert_int_equal(trie_get_child_index(node, L'f',0,3), 1);
    assert_int_equal(trie_get_child_index(node, L'f',0,4), 1);
    assert_int_equal(trie_get_child_index(node, L'f',1,1), 1);
    assert_int_equal(trie_get_child_index(node, L'f',1,2), 1);
    assert_int_equal(trie_get_child_index(node, L'f',1,3), 1);
    assert_int_equal(trie_get_child_index(node, L'f',1,4), 1);
    assert_int_equal(trie_get_child_index(node, L'f',2,2), 2);
    assert_int_equal(trie_get_child_index(node, L'f',2,3), 2);
    assert_int_equal(trie_get_child_index(node, L'f',2,4), 2);
    assert_int_equal(trie_get_child_index(node, L'f',3,3), 3);
    assert_int_equal(trie_get_child_index(node, L'f',3,4), 3);
    assert_int_equal(trie_get_child_index(node, L'f',4,4), 4);
    assert_int_equal(trie_get_child_index(node, L'g',0,0), 0);
    assert_int_equal(trie_get_child_index(node, L'g',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L'g',0,2), 2);
    assert_int_equal(trie_get_child_index(node, L'g',0,3), 2);
    assert_int_equal(trie_get_child_index(node, L'g',0,4), 2);
    assert_int_equal(trie_get_child_index(node, L'g',1,1), 1);
    assert_int_equal(trie_get_child_index(node, L'g',1,2), 2);
    assert_int_equal(trie_get_child_index(node, L'g',1,3), 2);
    assert_int_equal(trie_get_child_index(node, L'g',1,4), 2);
    assert_int_equal(trie_get_child_index(node, L'g',2,2), 2);
    assert_int_equal(trie_get_child_index(node, L'g',2,3), 2);
    assert_int_equal(trie_get_child_index(node, L'g',2,4), 2);
    assert_int_equal(trie_get_child_index(node, L'g',3,3), 3);
    assert_int_equal(trie_get_child_index(node, L'g',3,4), 3);
    assert_int_equal(trie_get_child_index(node, L'g',4,4), 4);
    assert_int_equal(trie_get_child_index(node, L'h',0,0), 0);
    assert_int_equal(trie_get_child_index(node, L'h',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L'h',0,2), 2);
    assert_int_equal(trie_get_child_index(node, L'h',0,3), 2);
    assert_int_equal(trie_get_child_index(node, L'h',0,4), 2);
    assert_int_equal(trie_get_child_index(node, L'h',1,1), 1);
    assert_int_equal(trie_get_child_index(node, L'h',1,2), 2);
    assert_int_equal(trie_get_child_index(node, L'h',1,3), 2);
    assert_int_equal(trie_get_child_index(node, L'h',1,4), 2);
    assert_int_equal(trie_get_child_index(node, L'h',2,2), 2);
    assert_int_equal(trie_get_child_index(node, L'h',2,3), 2);
    assert_int_equal(trie_get_child_index(node, L'h',2,4), 2);
    assert_int_equal(trie_get_child_index(node, L'h',3,3), 3);
    assert_int_equal(trie_get_child_index(node, L'h',3,4), 3);
    assert_int_equal(trie_get_child_index(node, L'h',4,4), 4);
    assert_int_equal(trie_get_child_index(node, L'o',0,0), 0);
    assert_int_equal(trie_get_child_index(node, L'o',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L'o',0,2), 2);
    assert_int_equal(trie_get_child_index(node, L'o',0,3), 3);
    assert_int_equal(trie_get_child_index(node, L'o',0,4), 3);
    assert_int_equal(trie_get_child_index(node, L'o',1,1), 1);
    assert_int_equal(trie_get_child_index(node, L'o',1,2), 2);
    assert_int_equal(trie_get_child_index(node, L'o',1,3), 3);
    assert_int_equal(trie_get_child_index(node, L'o',1,4), 3);
    assert_int_equal(trie_get_child_index(node, L'o',2,2), 2);
    assert_int_equal(trie_get_child_index(node, L'o',2,3), 3);
    assert_int_equal(trie_get_child_index(node, L'o',2,4), 3);
    assert_int_equal(trie_get_child_index(node, L'o',3,3), 3);
    assert_int_equal(trie_get_child_index(node, L'o',3,4), 3);
    assert_int_equal(trie_get_child_index(node, L'o',4,4), 4);
    assert_int_equal(trie_get_child_index(node, L's',0,0), 0);
    assert_int_equal(trie_get_child_index(node, L's',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L's',0,2), 2);
    assert_int_equal(trie_get_child_index(node, L's',0,3), 3);
    assert_int_equal(trie_get_child_index(node, L's',0,4), 3);
    assert_int_equal(trie_get_child_index(node, L's',1,1), 1);
    assert_int_equal(trie_get_child_index(node, L's',1,2), 2);
    assert_int_equal(trie_get_child_index(node, L's',1,3), 3);
    assert_int_equal(trie_get_child_index(node, L's',1,4), 3);
    assert_int_equal(trie_get_child_index(node, L's',2,2), 2);
    assert_int_equal(trie_get_child_index(node, L's',2,3), 3);
    assert_int_equal(trie_get_child_index(node, L's',2,4), 3);
    assert_int_equal(trie_get_child_index(node, L's',3,3), 3);
    assert_int_equal(trie_get_child_index(node, L's',3,4), 3);
    assert_int_equal(trie_get_child_index(node, L's',4,4), 4);
    assert_int_equal(trie_get_child_index(node, L'z',0,0), 0);
    assert_int_equal(trie_get_child_index(node, L'z',0,1), 1);
    assert_int_equal(trie_get_child_index(node, L'z',0,2), 2);
    assert_int_equal(trie_get_child_index(node, L'z',0,3), 3);
    assert_int_equal(trie_get_child_index(node, L'z',0,4), 4);
    assert_int_equal(trie_get_child_index(node, L'z',1,1), 1);
    assert_int_equal(trie_get_child_index(node, L'z',1,2), 2);
    assert_int_equal(trie_get_child_index(node, L'z',1,3), 3);
    assert_int_equal(trie_get_child_index(node, L'z',1,4), 4);
    assert_int_equal(trie_get_child_index(node, L'z',2,2), 2);
    assert_int_equal(trie_get_child_index(node, L'z',2,3), 3);
    assert_int_equal(trie_get_child_index(node, L'z',2,4), 4);
    assert_int_equal(trie_get_child_index(node, L'z',3,3), 3);
    assert_int_equal(trie_get_child_index(node, L'z',3,4), 4);
    assert_int_equal(trie_get_child_index(node, L'z',4,4), 4);
}

/**
 * Testuje pobranie dziecka o danej etykiecie
 * na pojedynczym węźle.
 */
static void trie_get_child_priv_empty_test(void **state)
{
    struct trie_node *node = *state;
    assert_true(trie_get_child_priv(node, L'a') == NULL);
}

/**
 * Testuje pobranie dziecka o danej etykiecie
 * na węźle z jednym dzieckiem.
 */
static void trie_get_child_priv_1_test(void **state)
{
    struct trie_node *node = *state;
    assert_true(trie_get_child_priv(node, L'a') == NULL);
    assert_true(trie_get_child_priv(node, L'c') == node->chd[0]);
    assert_true(trie_get_child_priv(node, L'k') == NULL);
}

/**
 * Testuje pobranie dziecka o danej etykiecie
 * na węźle z dwoma dziećmi.
 */
static void trie_get_child_priv_2_test(void **state)
{
    struct trie_node *node = *state;
    assert_true(trie_get_child_priv(node, L'a') == NULL);
    assert_true(trie_get_child_priv(node, L'd') == node->chd[0]);
    assert_true(trie_get_child_priv(node, L'i') == NULL);
    assert_true(trie_get_child_priv(node, L's') == node->chd[1]);
    assert_true(trie_get_child_priv(node, L'u') == NULL);
}

/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na pojedynczym węźle.
 */
static void trie_get_child_or_add_empty_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *child = trie_get_child_or_add_empty(node, L'h');
    assert_true(node->cnt == 1);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == child);
    assert_true(child->val == L'h');
    trie_done(node);
}

/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z jednym dzieckiem.
 * Zostanie dodane dziecko przed już istniejącym dzieckiem.
 */
static void trie_get_child_or_add_1A_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'c');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L'a');
    assert_true(node->cnt == 2);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == child);
    assert_true(node->chd[1] == c1);
    assert_true(child->val == L'a');
    trie_done(node);
}

/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z jednym dzieckiem.
 * Zostanie zwrócone istniejące dziecko.
 */
static void trie_get_child_or_add_1B_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'c');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L'c');
    assert_true(node->cnt == 1);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == child);
    assert_true(child == c1);
    trie_done(node);
}

/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z jednym dzieckiem.
 * Zostanie dodane dziecko po już istniejącym dziecku.
 */
static void trie_get_child_or_add_1C_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'c');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L'z');
    assert_true(node->cnt == 2);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == c1);
    assert_true(node->chd[1] == child);
    assert_true(child->val == L'z');
    trie_done(node);
}

/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z dwoma dziećmi.
 * Zostanie dodane dziecko przed już istniejącymi dziećmi.
 */
static void trie_get_child_or_add_2A_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'd');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L's');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L'a');
    assert_true(node->cnt == 3);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == child);
    assert_true(node->chd[1] == c1);
    assert_true(node->chd[2] == c2);
    assert_true(child->val == L'a');
    trie_done(node);
}
/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z dwoma dziećmi.
 * Zostanie zwrócone pierwsze dziecko.
 */
static void trie_get_child_or_add_2B_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'd');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L's');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L'd');
    assert_true(node->cnt == 2);
    assert_true(node->cap >= node->cnt);
    assert_true(c1 == child);
    assert_true(node->chd[0] == c1);
    assert_true(node->chd[1] == c2);
    assert_true(child->val == L'd');
    trie_done(node);
}
/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z dwoma dziećmi.
 * Zostanie dodane dziecko pomiędzy istniejącymi dziećmi.
 */
static void trie_get_child_or_add_2C_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'd');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L's');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L'h');
    assert_true(node->cnt == 3);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == c1);
    assert_true(node->chd[1] == child);
    assert_true(node->chd[2] == c2);
    assert_true(child->val == L'h');
    trie_done(node);
}
/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z dwoma dziećmi.
 * Zostanie zwrócone drugie istniejące dziecko.
 */
static void trie_get_child_or_add_2D_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'd');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L's');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L's');
    assert_true(node->cnt == 2);
    assert_true(node->cap >= node->cnt);
    assert_true(c2 == child);
    assert_true(node->chd[0] == c1);
    assert_true(node->chd[1] == c2);
    assert_true(child->val == L's');
    trie_done(node);
}
/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z dwoma dziećmi.
 * Zostanie dodane dziecko po drugim istniejącym dziecku.
 */
static void trie_get_child_or_add_2E_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'd');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L's');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L'w');
    assert_true(node->cnt == 3);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == c1);
    assert_true(node->chd[1] == c2);
    assert_true(node->chd[2] == child);
    assert_true(child->val == L'w');
    trie_done(node);
}
/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z trójką dzieci.
 * Zostanie dodane dziecko pomiędzy pierwsze a drugie dziecko.
 */
static void trie_get_child_or_add_3_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'd');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L's');
    struct trie_node *c3 = trie_get_child_or_add_empty(node, L'w');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L'f');
    assert_true(node->cnt == 4);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == c1);
    assert_true(node->chd[1] == child);
    assert_true(node->chd[2] == c2);
    assert_true(node->chd[3] == c3);
    assert_true(child->val == L'f');
    trie_done(node);
}

/**
 * Testuje pobranie lub dodanie dziecka o danej etykiecie
 * na węźle z czterema dziećmi.
 * Zostanie dodane dziecko pomiędzy drugie a trzecie dziecko.
 * Nastąpi powiększenie tablicy dzieci.
 */
static void trie_get_child_or_add_4copy_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'd');
    struct trie_node *c3 = trie_get_child_or_add_empty(node, L's');
    struct trie_node *c4 = trie_get_child_or_add_empty(node, L'w');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L'f');
    // Known state...
    struct trie_node *child = trie_get_child_or_add_empty(node, L'o');
    assert_true(node->cnt == 5);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == c1);
    assert_true(node->chd[1] == c2);
    assert_true(node->chd[2] == child);
    assert_true(node->chd[3] == c3);
    assert_true(node->chd[4] == c4);
    assert_true(child->val == L'o');
    trie_done(node);
}

/**
 * Testuje czyszczenie drzewa (root)->(h)->(p) bez żadnych liści
 * na poziomie (root)->(h).
 * Nic nie zostanie zmienione, bo (h) ma dziecko.
 */
static void trie_cleanup_11_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'h');
    struct trie_node *gch = trie_get_child_or_add_empty(chd, L'p');
    trie_cleanup(chd, node);
    assert_true(node->cnt == 1);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == chd);
    assert_true(chd->cnt == 1);
    assert_true(chd->cap >= chd->cnt);
    assert_true(chd->chd[0] == gch);
    trie_done(node);
}

/**
 * Testuje czyszczenie drzewa (root)->(h) z liściem w (h)
 * na poziomie (root)->(h).
 * Nic nie zostanie zmienione, bo (h) jest liściem.
 */
static void trie_cleanup_1_leaf_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'h');
    chd->leaf = 1;
    trie_cleanup(chd, node);
    assert_true(node->cnt == 1);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == chd);
    assert_true(chd->leaf != 0);
    assert_true(chd->cnt == 0);
    trie_done(node);
}

/**
 * Testuje czyszczenie drzewa (root)->(h) bez żadnych liści
 * na poziomie (root)->(h).
 * Zostanie usunięte jedyne dziecko (root),
 * bo nie jest liściem i nie ma dzieci.
 * Tablica dzieci w (root) zostanie usunięta.
 */
static void trie_cleanup_1_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'h');
    assert_true(chd->leaf == 0);
    trie_cleanup(chd, node);
    assert_true(node->cnt == 0);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd == NULL);
    trie_done(node);
}

/**
 * Testuje czyszczenie drzewa (root)->(h),(q) bez żadnych liści
 * na poziomie (root)->(h).
 * Zostanie usunięte (h), bo nie jest liściem i nie ma dzieci,
 * a (root) będzie miał tylko jedno dziecko.
 */
static void trie_cleanup_2A_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'h');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L'q');
    assert_true(c1->leaf == 0);
    trie_cleanup(c1, node);
    assert_true(node->cnt == 1);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == c2);
    trie_done(node);
}

/**
 * Testuje czyszczenie drzewa (root)->(h),(q) bez żadnych liści
 * na poziomie (root)->(q).
 * Zostanie usunięte (q), bo nie jest liściem i nie ma dzieci,
 * a (root) będzie miał tylko jedno dziecko.
 */
static void trie_cleanup_2B_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'h');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L'q');
    assert_true(c2->leaf == 0);
    trie_cleanup(c2, node);
    assert_true(node->cnt == 1);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == c1);
    trie_done(node);
}

/**
 * Testuje czyszczenie drzewa (root)->(b),(d),(f),(h),(j),(l),(n),(p) bez żadnych liści
 * na poziomie kolejno (root)->(l), (root)->(d), (root)->(n), (root)->(b), (root)->(p),
 * (root)->(h), (root)->(f).
 * Zostanią usunięte (b),(d),(f),(h),(l),(n),(p), bo nie są liśćmi i nie mają dzieci,
 * a (root) będzie miał tylko jedno dziecko.
 * Przy ostatnim clean-up'ie tablica dzieci (root)'a zostanie skrócona do 4 elementów.
 */
static void trie_cleanup_8_shrink_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'b');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L'd');
    struct trie_node *c3 = trie_get_child_or_add_empty(node, L'f');
    struct trie_node *c4 = trie_get_child_or_add_empty(node, L'h');
    struct trie_node *c5 = trie_get_child_or_add_empty(node, L'j');
    struct trie_node *c6 = trie_get_child_or_add_empty(node, L'l');
    struct trie_node *c7 = trie_get_child_or_add_empty(node, L'n');
    struct trie_node *c8 = trie_get_child_or_add_empty(node, L'p');
    assert_true(c1->leaf == 0);
    assert_true(c2->leaf == 0);
    assert_true(c3->leaf == 0);
    assert_true(c4->leaf == 0);
    assert_true(c6->leaf == 0);
    assert_true(c7->leaf == 0);
    assert_true(c8->leaf == 0);
    assert_true(node->cap == 8);
    trie_cleanup(c6, node);
    trie_cleanup(c2, node);
    trie_cleanup(c7, node);
    trie_cleanup(c1, node);
    trie_cleanup(c8, node);
    trie_cleanup(c4, node);
    // Does shrinking...
    trie_cleanup(c3, node);
    assert_true(node->cnt == 1);
    assert_true(node->cap == 4);
    assert_true(node->chd[0] == c5);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą usuwania słowa na drzewie (root)->(h) bez liści
 * podając do usunięcia puste słowo.
 * Nic się nie stanie, bo (h) nie jest liściem.
 */
static void trie_delete_helper_1_noleaf_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'h');
    assert_int_equal(trie_delete_helper(chd, node, L""), 0);
    assert_true(node->cnt == 1);
    assert_true(node->chd[0] == chd);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą usuwania słowa na drzewie (root)->(h)->(w)
 * z liściem w (h) podając do usunięcia puste słowo.
 * Węzeł (h) przestanie być liściem.
 */
static void trie_delete_helper_11_leaf_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'h');
    struct trie_node *gch = trie_get_child_or_add_empty(chd, L'w');
    chd->leaf = 1;
    assert_int_equal(trie_delete_helper(chd, node, L""), 1);
    assert_true(chd->leaf == 0);
    assert_true(node->cnt == 1);
    assert_true(node->chd[0] == chd);
    assert_true(chd->cnt == 1);
    assert_true(chd->chd[0] == gch);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą usuwania słowa na drzewie (root)->(h) bez liści
 * podając do usunięcia słowo "x".
 * Nic się nie stanie, bo (h) nie ma dziecka o etykiecie 'x'.
 */
static void trie_delete_helper_1_nochild_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'h');
    assert_int_equal(trie_delete_helper(chd, node, L"x"), 0);
    assert_true(node->cnt == 1);
    assert_true(node->chd[0] == chd);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą usuwania słowa na drzewie (root)->(h)->(w)
 * z liściem w (w) podając do usunięcia słowo "w".
 * Dziecko (root) zostanie usunięte.
 */
static void trie_delete_helper_11_leaf_A_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'h');
    struct trie_node *gch = trie_get_child_or_add_empty(chd, L'w');
    gch->leaf = 1;
    assert_int_equal(trie_delete_helper(chd, node, L"w"), 1);
    assert_true(node->cnt == 0);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą zapisującą drzewo (a)
 * dla formatu dictU.
 */
static void trie_serialize_formatU_helper_root_test(void **state)
{
    wwritep = 0;
    struct trie_node *node = trie_init();
    node->val = L'a';
    assert_int_equal(trie_serialize_formatU_helper(node, NULL), 0);
    assert_int_equal(wbuff[0], L'a');
    assert_int_equal(wbuff[1], 2);
    assert_int_equal(wwritep, 2);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą zapisującą drzewo (k)->(n)
 * dla formatu dictU.
 */
static void trie_serialize_formatU_helper_1_test(void **state)
{
    wwritep = 0;
    struct trie_node *node = trie_init();
    node->val = L'k';
    trie_get_child_or_add_empty(node, L'n');
    assert_int_equal(trie_serialize_formatU_helper(node, NULL), 0);
    assert_int_equal(wbuff[0], L'k');
    assert_int_equal(wbuff[1], L'n');
    assert_int_equal(wbuff[2], 2);
    assert_int_equal(wbuff[3], 2);
    assert_int_equal(wwritep, 4);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą zapisującą drzewo [k]->(n), gdzie k jest liściem
 * dla formatu dictU.
 */
static void trie_serialize_formatU_helper_1_leaf_test(void **state)
{
    wwritep = 0;
    struct trie_node *node = trie_init();
    node->val = L'k';
    node->leaf = 1;
    trie_get_child_or_add_empty(node, L'n');
    assert_int_equal(trie_serialize_formatU_helper(node, NULL), 0);
    assert_int_equal(wbuff[0], L'k');
    assert_int_equal(wbuff[1], 1);
    assert_int_equal(wbuff[2], L'n');
    assert_int_equal(wbuff[3], 2);
    assert_int_equal(wbuff[4], 2);
    assert_int_equal(wwritep, 5);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą zapisującą drzewo (k)->(n),(t)
 * dla formatu dictU
 */
static void trie_serialize_formatU_helper_2_test(void **state)
{
    wwritep = 0;
    struct trie_node *node = trie_init();
    node->val = L'k';
    trie_get_child_or_add_empty(node, L'n');
    trie_get_child_or_add_empty(node, L't');
    assert_int_equal(trie_serialize_formatU_helper(node, NULL), 0);
    assert_int_equal(wbuff[0], L'k');
    assert_int_equal(wbuff[1], L'n');
    assert_int_equal(wbuff[2], 2);
    assert_int_equal(wbuff[3], L't');
    assert_int_equal(wbuff[4], 2);
    assert_int_equal(wbuff[5], 2);
    assert_int_equal(wwritep, 6);
    trie_done(node);
}

/**
 * Testuje funkcję zapisującą drzewo (k)->(n),(t) w formacie dictU.
 */
static void trie_serialize_formatU_test(void **state)
{
    wwritep = 0;
    struct trie_node *node = trie_init();
    node->val = L'k';
    trie_get_child_or_add_empty(node, L'n');
    trie_get_child_or_add_empty(node, L't');
    assert_int_equal(trie_serialize_formatU(node, NULL), 0);
    assert_int_equal(wbuff[0], L'n');
    assert_int_equal(wbuff[1], 2);
    assert_int_equal(wbuff[2], L't');
    assert_int_equal(wbuff[3], 2);
    assert_int_equal(wbuff[4], 2);
    assert_int_equal(wwritep, 5);
    trie_done(node);
}


/**
 * Testuje funkcję pomocniczą wczytującą drzewo (root)->[t] w formacie dictU.
 */
static void trie_deserialize_formatU_helper_1_test(void **state)
{
    struct trie_node *node = trie_init();
    wbuff[0] = L't';
    wbuff[1] = 1;
    wbuff[2] = 2;
    wbuff[3] = 2;
    wreadp = 0;
    wfilelen = 4;
    assert_int_equal(trie_deserialize_formatU_helper(node, NULL), 0);
    assert_int_equal(node->cnt, 1);
    assert_int_equal(node->leaf, 0);
    assert_true(node->chd[0]->val == L't');
    assert_true(node->chd[0]->leaf != 0);
    assert_true(node->chd[0]->cnt == 0);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą wczytującą drzewo (root)->[t]->[k] w formacie dictU.
 */
static void trie_deserialize_formatU_helper_2_test(void **state)
{
    struct trie_node *node = trie_init();
    wbuff[0] = L't';
    wbuff[1] = 1;
    wbuff[2] = L'k';
    wbuff[3] = 1;
    wbuff[4] = 2;
    wbuff[5] = 2;
    wbuff[6] = 2;
    wreadp = 0;
    wfilelen = 7;
    assert_int_equal(trie_deserialize_formatU_helper(node, NULL), 0);
    assert_int_equal(node->cnt, 1);
    assert_int_equal(node->leaf, 0);
    assert_true(node->chd[0]->val == L't');
    assert_true(node->chd[0]->leaf != 0);
    assert_true(node->chd[0]->cnt == 1);
    assert_true(node->chd[0]->chd[0]->val == L'k');
    assert_true(node->chd[0]->chd[0]->leaf != 0);
    assert_true(node->chd[0]->chd[0]->cnt == 0);
    trie_done(node);
}

/**
 * Testuje funkcję pomocniczą wczytującą drzewo (root)->(t)->(k)->(n)->[k] w formacie dictU.
 */
static void trie_deserialize_formatU_helper_3_test(void **state)
{
    struct trie_node *node = trie_init();
    wbuff[0] = L't';
    wbuff[1] = L'k';
    wbuff[2] = L'n';
    wbuff[3] = L'k';
    wbuff[4] = 1;
    wbuff[5] = 2;
    wbuff[6] = 2;
    wbuff[7] = 2;
    wbuff[8] = 2;
    wbuff[9] = 2;
    wreadp = 0;
    wfilelen = 10;
    assert_int_equal(trie_deserialize_formatU_helper(node, NULL), 0);
    assert_int_equal(node->cnt, 1);
    assert_int_equal(node->leaf, 0);
    assert_true(node->chd[0]->val == L't');
    assert_true(node->chd[0]->leaf == 0);
    assert_true(node->chd[0]->cnt == 1);
    assert_true(node->chd[0]->chd[0]->val == L'k');
    assert_true(node->chd[0]->chd[0]->leaf == 0);
    assert_true(node->chd[0]->chd[0]->cnt == 1);
    assert_true(node->chd[0]->chd[0]->chd[0]->val == L'n');
    assert_true(node->chd[0]->chd[0]->chd[0]->leaf == 0);
    assert_true(node->chd[0]->chd[0]->chd[0]->cnt == 1);
    assert_true(node->chd[0]->chd[0]->chd[0]->chd[0]->val == L'k');
    assert_true(node->chd[0]->chd[0]->chd[0]->chd[0]->leaf != 0);
    assert_true(node->chd[0]->chd[0]->chd[0]->chd[0]->cnt == 0);
    trie_done(node);
}

/**
 * Testuje funkcję wczytującą drzewo (root)->[n],[t] w formacie dictU.
 */
static void trie_deserialize_fromatU_test(void **state)
{
    wbuff[0] = L'n';
    wbuff[1] = 1;
    wbuff[2] = 2;
    wbuff[3] = L't';
    wbuff[4] = 1;
    wbuff[5] = 2;
    wbuff[6] = 2;
    wreadp = 0;
    wfilelen = 7;
    struct trie_node *node = trie_deserialize_formatU(NULL);
    assert_int_equal(node->cnt, 2);
    assert_int_equal(node->leaf, 0);
    assert_int_equal(node->chd[0]->cnt, 0);
    assert_int_equal(node->chd[0]->leaf, 1);
    assert_int_equal(node->chd[0]->val, L'n');
    assert_int_equal(node->chd[1]->cnt, 0);
    assert_int_equal(node->chd[1]->leaf, 1);
    assert_int_equal(node->chd[1]->val, L't');
    trie_done(node);
}

/**
 * Testuje funkcję usuwającą zawartość drzewa na pojedynczym węźle.
 */
static void trie_clear_1_test(void **state)
{
    struct trie_node *node = trie_init();
    trie_clear(node);
    assert_int_equal(node->cnt, 0);
    trie_done(node);
}

/**
 * Testuje funkcję usuwającą zawartość drzewa na drzewie (root)->((f)->(d),(s)),((p)->(b))
 */
static void trie_clear_2_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'f');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L'p');
    trie_get_child_or_add_empty(c1, L'd');
    trie_get_child_or_add_empty(c1, L's');
    trie_get_child_or_add_empty(c2, L'b');
    trie_clear(node);
    assert_int_equal(node->cnt, 0);
    trie_done(node);
}

/**
 * Testuje funkcję wstawiającą element do drzewa dla (root) i słowa "x".
 */
static void trie_insert_1_test(void **state)
{
    struct trie_node *node = trie_init();
    assert_int_equal(trie_insert(node, L"x"), 1);
    assert_int_equal(node->cnt, 1);
    assert_int_equal(node->chd[0]->val, L'x');
    assert_int_equal(node->chd[0]->cnt, 0);
    assert_int_not_equal(node->chd[0]->leaf, 0);
    trie_done(node);
}

/**
 * Testuje funkcję wstawiającą element do drzewa dla (root)->(f) i słowa "f".
 * (f) nie jest liściem.
 */
static void trie_insert_2_test(void **state)
{
    struct trie_node *node = trie_init();
    trie_get_child_or_add_empty(node, L'f');
    assert_int_equal(trie_insert(node, L"f"),1);
    assert_int_equal(node->cnt, 1);
    assert_int_equal(node->chd[0]->val, L'f');
    assert_int_equal(node->chd[0]->cnt, 0);
    assert_int_not_equal(node->chd[0]->leaf, 0);
    trie_done(node);
}

/**
 * Testuje funkcję wstawiającą element do drzewa dla (root)->[f] i słowa "f".
 * [f] jest liściem.
 */
static void trie_insert_3_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *child = trie_get_child_or_add_empty(node, L'f');
    child->leaf = 1;
    assert_int_equal(trie_insert(node, L"f"),0);
    assert_int_equal(node->cnt, 1);
    assert_int_equal(node->chd[0]->val, L'f');
    assert_int_equal(node->chd[0]->cnt, 0);
    assert_int_not_equal(node->chd[0]->leaf, 0);
    trie_done(node);
}

/**
 * Testuje funkcję wstawiającą element do drzewa dla (root)->(f) i słowa "fl".
 */
static void trie_insert_4_test(void **state)
{
    struct trie_node *node = trie_init();
    trie_get_child_or_add_empty(node, L'f');
    assert_int_equal(trie_insert(node, L"fl"),1);
    assert_int_equal(node->cnt, 1);
    assert_int_equal(node->chd[0]->val, L'f');
    assert_int_equal(node->chd[0]->cnt, 1);
    assert_int_equal(node->chd[0]->leaf, 0);
    assert_int_equal(node->chd[0]->chd[0]->val, L'l');
    assert_int_equal(node->chd[0]->chd[0]->cnt, 0);
    assert_int_equal(node->chd[0]->chd[0]->leaf, 1);
    trie_done(node);
}

/**
 * Testuje funkcję zajdującą dla drzewa (root) i słowa "".
 */
static void trie_find_1_test(void **state)
{
    struct trie_node *node = trie_init();
    assert_int_equal(trie_find(node, L""),0);
    trie_done(node);
}

/**
 * Testuje funkcję zajdującą dla drzewa [root] i słowa "".
 */
static void trie_find_2_test(void **state)
{
    struct trie_node *node = trie_init();
    node->leaf = 1;
    assert_int_equal(trie_find(node, L""),1);
    trie_done(node);
}

/**
 * Testuje funkcję zajdującą dla drzewa (root) i słowa "n".
 */
static void trie_find_3_test(void **state)
{
    struct trie_node *node = trie_init();
    assert_int_equal(trie_find(node, L"n"),0);
    trie_done(node);
}

/**
 * Testuje funkcję zajdującą dla drzewa (root)->(f) i słowa "f".
 */
static void trie_find_4_test(void **state)
{
    struct trie_node *node = trie_init();
    trie_get_child_or_add_empty(node, L'f');
    assert_int_equal(trie_find(node, L"f"),0);
    trie_done(node);
}

/**
 * Testuje funkcję zajdującą dla drzewa (root)->[f] i słowa "f".
 */
static void trie_find_5_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *child = trie_get_child_or_add_empty(node, L'f');
    child->leaf = 1;
    assert_int_equal(trie_find(node, L"f"),1);
    trie_done(node);
}

/**
 * Testuje funkcję usuwającą dla drzewa (root) i słowa "e".
 */
static void trie_delete_1_test(void **state)
{
    struct trie_node *node = trie_init();
    assert_int_equal(trie_delete(node, L"e"), 0);
    trie_done(node);
}

/**
 * Testuje funkcję usuwającą dla drzewa (root)->(f) i słowa "f".
 */
static void trie_delete_2_test(void **state)
{
    struct trie_node *node = trie_init();
    trie_get_child_or_add_empty(node, L'f');
    assert_int_equal(trie_delete(node, L"f"), 0);
    trie_done(node);
}

/**
 * Testuje funkcję usuwającą dla drzewa (root)->[f] i słowa "f".
 */
static void trie_delete_3_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *child = trie_get_child_or_add_empty(node, L'f');
    child->leaf = 1;
    assert_int_equal(trie_delete(node, L"f"), 1);
    trie_done(node);
}

/**
 * Testuje funkcję zapisującą drzewo dla {gl, gr, p}.
 */
static void trie_serialize_test(void **state)
{
    struct trie_node *node = trie_init();
    trie_insert(node, L"p");
    trie_insert(node, L"gl");
    trie_insert(node, L"gr");
    wchar_t *output = L"gl\x01\x02r\x01\x02\x02p\x01\x02\x02";
    wwritep = 0;
    trie_serialize(node, NULL);
    assert_memory_equal(wbuff, output, 12*sizeof(wchar_t));
    trie_done(node);
}

/**
 * Testuje pobranie dziecka o danej etykiecie
 * na pojedynczym węźle.
 */
static void trie_get_child_empty_test(void **state)
{
    struct trie_node *node = *state;
    assert_true(trie_get_child(node, L'a') == NULL);
}

/**
 * Testuje pobranie dziecka o danej etykiecie
 * na węźle z jednym dzieckiem.
 */
static void trie_get_child_1_test(void **state)
{
    struct trie_node *node = *state;
    assert_true(trie_get_child(node, L'a') == NULL);
    assert_true(trie_get_child(node, L'c') == node->chd[0]);
    assert_true(trie_get_child(node, L'k') == NULL);
}

/**
 * Testuje pobranie dziecka o danej etykiecie
 * na węźle z dwoma dziećmi.
 */
static void trie_get_child_2_test(void **state)
{
    struct trie_node *node = *state;
    assert_true(trie_get_child(node, L'a') == NULL);
    assert_true(trie_get_child(node, L'd') == node->chd[0]);
    assert_true(trie_get_child(node, L'i') == NULL);
    assert_true(trie_get_child(node, L's') == node->chd[1]);
    assert_true(trie_get_child(node, L'u') == NULL);
}

/**
 * Testuje funkcję wczytujące drzewo {gl, gr, p}.
 */
static void trie_deserialize_test(void **state)
{
    wchar_t *output = L"gl\x01\x02r\x01\x02\x02p\x01\x02\x02";
    memcpy(wbuff, output, 12*sizeof(wchar_t));
    wreadp = 0;
    wfilelen = 12;
    struct trie_node * node = trie_deserialize(NULL);
    assert_int_equal(node->cnt, 2);
    assert_int_equal(node->chd[0]->val, L'g');
    assert_int_equal(node->chd[0]->cnt, 2);
    assert_false(node->chd[0]->leaf);
    assert_int_equal(node->chd[1]->val, L'p');
    assert_int_equal(node->chd[1]->cnt, 0);
    assert_true(node->chd[1]->leaf);
    
    assert_int_equal(node->chd[0]->chd[0]->val, L'l');
    assert_int_equal(node->chd[0]->chd[0]->cnt, 0);
    assert_true(node->chd[0]->chd[0]->leaf);
    assert_int_equal(node->chd[0]->chd[1]->val, L'r');
    assert_int_equal(node->chd[0]->chd[1]->cnt, 0);
    assert_true(node->chd[0]->chd[1]->leaf);
    
    trie_done(node);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(trie_init_done_test),
        cmocka_unit_test_setup_teardown(trie_get_child_index_empty_test, node_0_setup, node_0_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_index_1_test, node_1_setup, node_1_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_index_2_test, node_2_setup, node_2_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_index_4_test, node_4_setup, node_4_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_priv_empty_test, node_0_setup, node_0_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_priv_1_test, node_1_setup, node_1_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_priv_2_test, node_2_setup, node_2_teardown),
        cmocka_unit_test(trie_get_child_or_add_empty_test),
        cmocka_unit_test(trie_get_child_or_add_1A_test),
        cmocka_unit_test(trie_get_child_or_add_1B_test),
        cmocka_unit_test(trie_get_child_or_add_1C_test),
        cmocka_unit_test(trie_get_child_or_add_2A_test),
        cmocka_unit_test(trie_get_child_or_add_2B_test),
        cmocka_unit_test(trie_get_child_or_add_2C_test),
        cmocka_unit_test(trie_get_child_or_add_2D_test),
        cmocka_unit_test(trie_get_child_or_add_2E_test),
        cmocka_unit_test(trie_get_child_or_add_3_test),
        cmocka_unit_test(trie_get_child_or_add_4copy_test),
        cmocka_unit_test(trie_cleanup_11_test),
        cmocka_unit_test(trie_cleanup_1_leaf_test),
        cmocka_unit_test(trie_cleanup_1_test),
        cmocka_unit_test(trie_cleanup_2A_test),
        cmocka_unit_test(trie_cleanup_2B_test),
        cmocka_unit_test(trie_cleanup_8_shrink_test),
        cmocka_unit_test(trie_delete_helper_1_noleaf_test),
        cmocka_unit_test(trie_delete_helper_11_leaf_test),
        cmocka_unit_test(trie_delete_helper_1_nochild_test),
        cmocka_unit_test(trie_delete_helper_11_leaf_A_test),
        cmocka_unit_test(trie_serialize_formatU_helper_root_test),
        cmocka_unit_test(trie_serialize_formatU_helper_1_test),
        cmocka_unit_test(trie_serialize_formatU_helper_1_leaf_test),
        cmocka_unit_test(trie_serialize_formatU_helper_2_test),
        cmocka_unit_test(trie_serialize_formatU_test),
        cmocka_unit_test(trie_deserialize_formatU_helper_1_test),
        cmocka_unit_test(trie_deserialize_formatU_helper_2_test),
        cmocka_unit_test(trie_deserialize_formatU_helper_3_test),
        cmocka_unit_test(trie_deserialize_fromatU_test),
        cmocka_unit_test(trie_clear_1_test),
        cmocka_unit_test(trie_clear_2_test),
        cmocka_unit_test(trie_insert_1_test),
        cmocka_unit_test(trie_insert_2_test),
        cmocka_unit_test(trie_insert_3_test),
        cmocka_unit_test(trie_insert_4_test),
        cmocka_unit_test(trie_find_1_test),
        cmocka_unit_test(trie_find_2_test),
        cmocka_unit_test(trie_find_3_test),
        cmocka_unit_test(trie_find_4_test),
        cmocka_unit_test(trie_find_5_test),
        cmocka_unit_test(trie_delete_1_test),
        cmocka_unit_test(trie_delete_2_test),
        cmocka_unit_test(trie_delete_3_test),
        cmocka_unit_test(trie_serialize_test),
        cmocka_unit_test(trie_deserialize_test),
        cmocka_unit_test_setup_teardown(trie_get_child_empty_test, node_0_setup, node_0_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_1_test, node_1_setup, node_1_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_2_test, node_2_setup, node_2_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
