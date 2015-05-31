#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "trie.h"
#include "charmap.h"
#include "word_list.h"

#ifdef free
#undef free
#endif /* free */
#define free(ptr) _test_free(ptr, __FILE__, __LINE__)
void _test_free(void* const ptr, const char* file, const int line);


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
extern struct trie_node * trie_get_child(struct trie_node *node, wchar_t value);
extern struct trie_node * trie_get_child_or_add_empty(struct trie_node *node, wchar_t value);
extern void trie_cleanup(struct trie_node *node, struct trie_node *parent);
extern int trie_delete_helper(struct trie_node *node, struct trie_node *parent, const wchar_t *word);
extern void trie_fill_charmap(struct trie_node *node, struct char_map *map, wchar_t **trans, char *symbol, int length, int *maxlength);
extern void trie_serialize_formatA_ender(int res, int count, int loglen, FILE *file);
extern int trie_serialize_formatA_helper(struct trie_node *node, struct char_map *map, int count, int loglen, FILE *file);
extern void trie_serialize_formatA(struct trie_node *root, struct char_map *map, wchar_t *trans, int loglen, FILE *file);
extern int trie_deserialize_formatA_helper(struct trie_node *node, int lcount, int loglen, wchar_t * translator, FILE *file);
extern struct trie_node * trie_deserialize_formatA(FILE *file);
extern void fix_size(wchar_t **string, int length, int *capacity);
extern void trie_hints_helper(struct trie_node *node, const wchar_t *word,
                       wchar_t **created, int length, int *capacity,
                       int points, struct word_list *list);

static void trie_init_done_test(void **state)
{
    struct trie_node *node = trie_init();
    assert_true(node != NULL);
    assert_true(node->cnt == 0);
    assert_true(node->leaf == 0);
    trie_done(node);
}

static int node_0_setup(void **state)
{
    *state = trie_init();
    return 0;
}

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

static int node_0_teardown(void **state)
{
    struct trie_node *node = *state;
    assert_true(node->chd == NULL);
    assert_true(node->cnt == 0);
    free(node);
    return 0;
}

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

static void trie_get_child_index_empty_test(void **state)
{
    struct trie_node *node = *state;
    assert_int_equal(trie_get_child_index(node, L'a',0,0), -1);
}

static void trie_get_child_index_1_test(void **state)
{
    struct trie_node *node = *state;
    assert_int_equal(trie_get_child_index(node, L'c',0,1), 0);
}

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

static void trie_get_child_empty_test(void **state)
{
    struct trie_node *node = *state;
    assert_true(trie_get_child(node, L'a') == NULL);
}

static void trie_get_child_1_test(void **state)
{
    struct trie_node *node = *state;
    assert_true(trie_get_child(node, L'a') == NULL);
    assert_true(trie_get_child(node, L'c') == node->chd[0]);
    assert_true(trie_get_child(node, L'k') == NULL);
}

static void trie_get_child_2_test(void **state)
{
    struct trie_node *node = *state;
    assert_true(trie_get_child(node, L'a') == NULL);
    assert_true(trie_get_child(node, L'd') == node->chd[0]);
    assert_true(trie_get_child(node, L'i') == NULL);
    assert_true(trie_get_child(node, L's') == node->chd[1]);
    assert_true(trie_get_child(node, L'u') == NULL);
}

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

static void trie_delete_helper_1_noleaf_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'h');
    assert_int_equal(trie_delete_helper(chd, node, L""), 0);
    assert_true(node->cnt == 1);
    assert_true(node->chd[0] == chd);
    trie_done(node);
}

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

static void trie_delete_helper_1_nochild_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'h');
    assert_int_equal(trie_delete_helper(chd, node, L"x"), 0);
    assert_true(node->cnt == 1);
    assert_true(node->chd[0] == chd);
    trie_done(node);
}

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

static void trie_fill_charmap_root_test(void **state)
{
    struct trie_node *node = trie_init();
    node->val = L'v';
    struct char_map *map = char_map_init();
    wchar_t trans[256];
    wchar_t *tred = trans;
    char sym = 3;
    int ml = 0;
    trie_fill_charmap(node, map, &tred, &sym, 0, &ml);
    char os = 42;
    assert_true(char_map_get(map, L'v', &os) == 1);
    assert_true(os == 3);
    assert_true(char_map_size(map) == 1);
    assert_true(tred = trans+1);
    assert_true(trans[0] == L'v');
    assert_true(sym == 4);
    assert_true(ml == 0);
    char_map_done(map);
    trie_done(node);
}

static void trie_fill_charmap_1_test(void **state)
{
    struct trie_node *node = trie_init();
    node->val = L'v';
    struct trie_node *chd = trie_get_child_or_add_empty(node, L's');
    struct char_map *map = char_map_init();
    wchar_t trans[256];
    wchar_t *tred = trans;
    char sym = 3;
    int ml = 0;
    trie_fill_charmap(node, map, &tred, &sym, 0, &ml);
    char os = 42;
    assert_true(char_map_get(map, L'v', &os) == 1);
    assert_true(os == 3);
    assert_true(char_map_get(map, L's', &os) == 1);
    assert_true(os == 4);
    assert_true(char_map_size(map) == 2);
    assert_true(tred = trans+2);
    assert_true(trans[0] == L'v');
    assert_true(trans[1] == L's');
    assert_true(sym == 5);
    assert_true(ml == 1);
    char_map_done(map);
    trie_done(node);
}

static void trie_serialize_formatA_ender_1_test(void **state)
{
    unsigned char buff[8];
    buff[0] = 0;
    buff[1] = 0;
    FILE * file = fmemopen(buff, 8, "w");
    trie_serialize_formatA_ender(5, 42, 8, file);
    fclose(file);
    assert_int_equal(buff[0], 55);
    assert_int_equal(buff[1], 0);
}

static void trie_serialize_formatA_ender_2_test(void **state)
{
    unsigned char buff[8];
    buff[0] = 0;
    buff[1] = 0;
    buff[2] = 0;
    FILE * file = fmemopen(buff, 8, "w");
    trie_serialize_formatA_ender(36, 242, 8, file);
    fclose(file);
    assert_int_equal(buff[0], 245);
    assert_int_equal(buff[1], 248);
    assert_int_equal(buff[2], 0);
}

static void trie_serialize_formatA_helper_root_test(void **state)
{
    struct trie_node *node = trie_init();
    node->val = L'a';
    struct char_map *map = char_map_init();
    char_map_put(map, L'a', 5);
    unsigned char buff[8];
    memset(buff, 0, 8);
    FILE * file = fmemopen(buff, 8, "w");
    assert_int_equal(trie_serialize_formatA_helper(node, map, 12, 8, file), 1);
    fclose(file);
    assert_int_equal(buff[0], 5);
    assert_int_equal(buff[1], 0);
    char_map_done(map);
    trie_done(node);
}

static void trie_serialize_formatA_helper_1_test(void **state)
{
    struct trie_node *node = trie_init();
    node->val = L'k';
    struct trie_node *child = trie_get_child_or_add_empty(node, L'n');
    struct char_map *map = char_map_init();
    char_map_put(map, L'k', 5);
    char_map_put(map, L'n', 9);
    unsigned char buff[8];
    memset(buff, 0, 8);
    FILE * file = fmemopen(buff, 8, "w");
    assert_int_equal(trie_serialize_formatA_helper(node, map, 12, 8, file), 2);
    fclose(file);
    assert_int_equal(buff[0], 5);
    assert_int_equal(buff[1], 9);
    assert_int_equal(buff[2], 0);
    char_map_done(map);
    trie_done(node);
}

static void trie_serialize_formatA_helper_1_leaf_test(void **state)
{
    struct trie_node *node = trie_init();
    node->val = L'k';
    node->leaf = 1;
    struct trie_node *child = trie_get_child_or_add_empty(node, L'n');
    struct char_map *map = char_map_init();
    char_map_put(map, L'k', 5);
    char_map_put(map, L'n', 9);
    unsigned char buff[8];
    memset(buff, 0, 8);
    FILE * file = fmemopen(buff, 8, "w");
    assert_int_equal(trie_serialize_formatA_helper(node, map, 12, 8, file), 2);
    fclose(file);
    assert_int_equal(buff[0], 5);
    assert_int_equal(buff[1], 12);
    assert_int_equal(buff[2], 9);
    assert_int_equal(buff[3], 0);
    char_map_done(map);
    trie_done(node);
}

static void trie_serialize_formatA_helper_2_test(void **state)
{
    struct trie_node *node = trie_init();
    node->val = L'k';
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'n');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L't');
    struct char_map *map = char_map_init();
    char_map_put(map, L'k', 5);
    char_map_put(map, L'n', 9);
    char_map_put(map, L't', 2);
    unsigned char buff[8];
    memset(buff, 0, 8);
    FILE * file = fmemopen(buff, 8, "w");
    assert_int_equal(trie_serialize_formatA_helper(node, map, 12, 8, file), 2);
    fclose(file);
    assert_int_equal(buff[0], 5);
    assert_int_equal(buff[1], 9);
    assert_int_equal(buff[2], 21);
    assert_int_equal(buff[3], 2);
    assert_int_equal(buff[4], 0);
    char_map_done(map);
    trie_done(node);
}

static void trie_serialize_formatA_test(void **state)
{
    struct trie_node *node = trie_init();
    node->val = L'k';
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'n');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L't');
    struct char_map *map = char_map_init();
    char_map_put(map, L'k', 5);
    char_map_put(map, L'n', 9);
    char_map_put(map, L't', 2);
    char_map_put(map, L'a', 0);
    char_map_put(map, L'b', 1);
    char_map_put(map, L'c', 3);
    char_map_put(map, L'd', 4);
    char_map_put(map, L'e', 6);
    char_map_put(map, L'f', 7);
    char_map_put(map, L'g', 8);
    char_map_put(map, L'h', 10);
    char_map_put(map, L'i', 11);
    wchar_t trans[256];
    trans[0] = L'a';
    trans[1] = L'b';
    trans[2] = L't';
    trans[3] = L'c';
    trans[4] = L'd';
    trans[5] = L'k';
    trans[6] = L'e';
    trans[7] = L'f';
    trans[8] = L'g';
    trans[9] = L'n';
    trans[10] = L'h';
    trans[11] = L'i';
    unsigned char buff[64];
    memset(buff, 0, 64);
    FILE * file = fmemopen(buff, 64, "w");
    char *output = "dictA\x0c\x08\x00\x00\x00\x0c""abtcdkefgnhi\x09\x15\x02\x15\x0d";
    trie_serialize_formatA(node, map, trans, 8, file);
    fclose(file);
    assert_memory_equal(buff, output, 28);
    assert_int_equal(buff[28], 0);
    char_map_done(map);
    trie_done(node);
}

static void trie_deserialize_formatA_helper_1_test(void **state)
{
    struct trie_node *node = trie_init();
    unsigned char buff[64];
    memset(buff, 0, 64);
    buff[0] = 2;
    buff[1] = 22;
    wchar_t trans[256];
    trans[0] = L'a';
    trans[1] = L'b';
    trans[2] = L't';
    trans[3] = L'c';
    trans[4] = L'd';
    trans[5] = L'k';
    trans[6] = L'e';
    trans[7] = L'f';
    trans[8] = L'g';
    trans[9] = L'n';
    trans[10] = L'h';
    trans[11] = L'i';
    FILE * file = fmemopen(buff, 64, "r");
    assert_int_equal(trie_deserialize_formatA_helper(node, 12, 8, trans, file), 0);
    fclose(file);
    assert_int_equal(node->cnt, 1);
    assert_int_equal(node->leaf, 0);
    assert_true(node->chd[0]->val == L't');
    assert_true(node->chd[0]->leaf != 0);
    assert_true(node->chd[0]->cnt == 0);
    trie_done(node);
}

static void trie_deserialize_formatA_helper_2_test(void **state)
{
    struct trie_node *node = trie_init();
    unsigned char buff[64];
    memset(buff, 0, 64);
    buff[0] = 2;
    buff[1] = 12;
    buff[2] = 5;
    buff[3] = 25;
    wchar_t trans[256];
    trans[0] = L'a';
    trans[1] = L'b';
    trans[2] = L't';
    trans[3] = L'c';
    trans[4] = L'd';
    trans[5] = L'k';
    trans[6] = L'e';
    trans[7] = L'f';
    trans[8] = L'g';
    trans[9] = L'n';
    trans[10] = L'h';
    trans[11] = L'i';
    FILE * file = fmemopen(buff, 64, "r");
    assert_int_equal(trie_deserialize_formatA_helper(node, 12, 8, trans, file), 2);
    fclose(file);
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

static void trie_deserialize_formatA_helper_3_test(void **state)
{
    struct trie_node *node = trie_init();
    unsigned char buff[64];
    memset(buff, 0, 64);
    buff[0] = 2;
    buff[1] = 5;
    buff[2] = 9;
    buff[3] = 5;
    buff[4] = 13;
    buff[5] = 15;
    wchar_t trans[256];
    trans[0] = L'a';
    trans[1] = L'b';
    trans[2] = L't';
    trans[3] = L'c';
    trans[4] = L'd';
    trans[5] = L'k';
    trans[6] = L'e';
    trans[7] = L'f';
    trans[8] = L'g';
    trans[9] = L'n';
    trans[10] = L'h';
    trans[11] = L'i';
    FILE * file = fmemopen(buff, 64, "r");
    assert_int_equal(trie_deserialize_formatA_helper(node, 12, 8, trans, file), 0);
    fclose(file);
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

static void trie_deserialize_fromatA_test(void **state)
{
    unsigned char buff[64];
    char *output = "\x0c\x08\x00\x00\x00\x0c""abtcdkefgnhi\x09\x15\x02\x15\x0d";
    memcpy(buff, output, 29);
    FILE * file = fmemopen(buff, 64, "r");
    struct trie_node *node = trie_deserialize_formatA(file);
    fclose(file);
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

static void fix_size_1_test(void **state)
{
    wchar_t *str = malloc(4*sizeof(wchar_t));
    str[0] = L'a';
    str[1] = L'b';
    int cap = 4;
    fix_size(&str, 2, &cap);
    assert_int_equal(cap, 4);
    assert_memory_equal(str, L"ab", 2);
    free(str);
}

static void fix_size_2_test(void **state)
{
    wchar_t *str = malloc(4*sizeof(wchar_t));
    str[0] = L'a';
    str[1] = L'b';
    str[2] = L'c';
    str[3] = L'd';
    int cap = 4;
    fix_size(&str, 5, &cap);
    assert_int_equal(cap, 8);
    assert_memory_equal(str, L"abcd", 4);
    free(str);
}

static void trie_hints_helper_nochange_1_test(void **state)
{
    struct trie_node *node = trie_init();
    node->leaf = 1;
    struct word_list list;
    word_list_init(&list);
    wchar_t *created = malloc(4*sizeof(wchar_t));
    created[0] = L'a';
    int cap = 4;
    trie_hints_helper(node, L"", &created, 1, &cap, 0, &list);
    assert_int_equal(word_list_size(&list), 1);
    assert_true(wcsncmp(word_list_get(&list)[0], L"a", 2) == 0);
    word_list_done(&list);
    trie_done(node);
    free(created);
}
static void trie_hints_helper_nochange_2_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'f');
    chd->leaf = 1;
    struct word_list list;
    word_list_init(&list);
    wchar_t *created = malloc(4*sizeof(wchar_t));
    created[0] = L'a';
    int cap = 4;
    trie_hints_helper(node, L"f", &created, 1, &cap, 0, &list);
    assert_int_equal(word_list_size(&list), 1);
    assert_true(wcsncmp(word_list_get(&list)[0], L"af", 2) == 0);
    word_list_done(&list);
    trie_done(node);
    free(created);
}

static void trie_hints_helper_change_NC1_test(void **state)
{
    struct trie_node *node = trie_init();
    node->leaf = 1;
    struct word_list list;
    word_list_init(&list);
    wchar_t *created = malloc(4*sizeof(wchar_t));
    created[0] = L'a';
    int cap = 4;
    trie_hints_helper(node, L"", &created, 1, &cap, 1, &list);
    assert_int_equal(word_list_size(&list), 1);
    assert_true(wcsncmp(word_list_get(&list)[0], L"a", 2) == 0);
    word_list_done(&list);
    trie_done(node);
    free(created);
}
static void trie_hints_helper_change_NC2_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'f');
    chd->leaf = 1;
    struct word_list list;
    word_list_init(&list);
    wchar_t *created = malloc(4*sizeof(wchar_t));
    created[0] = L'a';
    int cap = 4;
    trie_hints_helper(node, L"f", &created, 1, &cap, 1, &list);
    assert_int_equal(word_list_size(&list), 1);
    assert_true(wcsncmp(word_list_get(&list)[0], L"af", 2) == 0);
    word_list_done(&list);
    trie_done(node);
    free(created);
}

static void trie_hints_helper_change_1_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'f');
    chd->leaf = 1;
    struct word_list list;
    word_list_init(&list);
    wchar_t *created = malloc(4*sizeof(wchar_t));
    created[0] = L'a';
    int cap = 4;
    trie_hints_helper(node, L"", &created, 1, &cap, 1, &list);
    assert_int_equal(word_list_size(&list), 1);
    assert_true(wcsncmp(word_list_get(&list)[0], L"af", 2) == 0);
    word_list_done(&list);
    trie_done(node);
    free(created);
}

static void trie_hints_helper_change_2_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'f');
    chd->leaf = 1;
    struct word_list list;
    word_list_init(&list);
    wchar_t *created = malloc(4*sizeof(wchar_t));
    created[0] = L'a';
    int cap = 4;
    trie_hints_helper(node, L"x", &created, 1, &cap, 1, &list);
    assert_int_equal(word_list_size(&list), 1);
    assert_true(wcsncmp(word_list_get(&list)[0], L"af", 2) == 0);
    word_list_done(&list);
    trie_done(node);
    free(created);
}

static void trie_hints_helper_change_3_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *chd = trie_get_child_or_add_empty(node, L'f');
    chd->leaf = 1;
    struct word_list list;
    word_list_init(&list);
    wchar_t *created = malloc(4*sizeof(wchar_t));
    created[0] = L'a';
    int cap = 4;
    trie_hints_helper(node, L"f", &created, 1, &cap, 1, &list);
    assert_int_equal(word_list_size(&list), 1);
    assert_true(wcsncmp(word_list_get(&list)[0], L"af", 2) == 0);
    word_list_done(&list);
    trie_done(node);
    free(created);
}

static void trie_clear_1_test(void **state)
{
    struct trie_node *node = trie_init();
    trie_clear(node);
    assert_int_equal(node->cnt, 0);
    trie_done(node);
}

static void trie_clear_2_test(void **state)
{
    struct trie_node *node = trie_init();
    struct trie_node *c1 = trie_get_child_or_add_empty(node, L'f');
    struct trie_node *c2 = trie_get_child_or_add_empty(node, L'p');
    struct trie_node *g1 = trie_get_child_or_add_empty(c1, L'd');
    struct trie_node *g2 = trie_get_child_or_add_empty(c1, L's');
    struct trie_node *h1 = trie_get_child_or_add_empty(c2, L'b');
    trie_clear(node);
    assert_int_equal(node->cnt, 0);
    trie_done(node);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(trie_init_done_test),
        cmocka_unit_test_setup_teardown(trie_get_child_index_empty_test, node_0_setup, node_0_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_index_1_test, node_1_setup, node_1_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_index_2_test, node_2_setup, node_2_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_index_4_test, node_4_setup, node_4_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_empty_test, node_0_setup, node_0_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_1_test, node_1_setup, node_1_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_2_test, node_2_setup, node_2_teardown),
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
        cmocka_unit_test(trie_fill_charmap_root_test),
        cmocka_unit_test(trie_fill_charmap_1_test),
        cmocka_unit_test(trie_serialize_formatA_ender_1_test),
        cmocka_unit_test(trie_serialize_formatA_ender_2_test),
        cmocka_unit_test(trie_serialize_formatA_helper_root_test),
        cmocka_unit_test(trie_serialize_formatA_helper_1_test),
        cmocka_unit_test(trie_serialize_formatA_helper_1_leaf_test),
        cmocka_unit_test(trie_serialize_formatA_helper_2_test),
        cmocka_unit_test(trie_serialize_formatA_test),
        cmocka_unit_test(trie_deserialize_formatA_helper_1_test),
        cmocka_unit_test(trie_deserialize_formatA_helper_2_test),
        cmocka_unit_test(trie_deserialize_formatA_helper_3_test),
        cmocka_unit_test(trie_deserialize_fromatA_test),
        cmocka_unit_test(fix_size_1_test),
        cmocka_unit_test(fix_size_2_test),
        cmocka_unit_test(trie_hints_helper_nochange_1_test),
        cmocka_unit_test(trie_hints_helper_nochange_2_test),
        cmocka_unit_test(trie_hints_helper_change_NC1_test),
        cmocka_unit_test(trie_hints_helper_change_NC2_test),
        cmocka_unit_test(trie_hints_helper_change_1_test),
        cmocka_unit_test(trie_hints_helper_change_2_test),
        cmocka_unit_test(trie_hints_helper_change_3_test),
        cmocka_unit_test(trie_clear_1_test),
        cmocka_unit_test(trie_clear_2_test),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
