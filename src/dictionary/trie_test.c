#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "trie.h"

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

static int node_0_setup(void **state)
{
    struct trie_node *node = (struct trie_node *) malloc(sizeof(struct trie_node));
    node->cap = 0;
    node->cnt = 0;
    node->chd = NULL;
    *state = node;
    return 0;
}

static int node_1_setup(void **state)
{
    struct trie_node *node = malloc(sizeof(struct trie_node));
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
    struct trie_node *node = malloc(sizeof(struct trie_node));
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
    struct trie_node *node = malloc(sizeof(struct trie_node));
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

static int node_teardown(void **state)
{
    struct trie_node *node = *state;
    trie_done(node);
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
    struct trie_node *node = *state;
    struct trie_node *child = trie_get_child_or_add_empty(node, L'h');
    assert_true(node->cnt == 1);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == child);
    assert_true(child->val == L'h');
    trie_done(node);
}

static void trie_get_child_or_add_1A_test(void **state)
{
    struct trie_node *node = *state;
    struct trie_node *child = trie_get_child_or_add_empty(node, L'a');
    assert_true(node->cnt == 2);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == child);
    assert_true(child->val == L'a');
    trie_done(node);
}

static void trie_get_child_or_add_1B_test(void **state)
{
    struct trie_node *node = *state;
    struct trie_node *child = trie_get_child_or_add_empty(node, L'c');
    assert_true(node->cnt == 1);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == child);
    assert_true(child->val == L'c');
    trie_done(node);
}
static void trie_get_child_or_add_1C_test(void **state)
{
    struct trie_node *node = *state;
    struct trie_node *child = trie_get_child_or_add_empty(node, L'z');
    assert_true(node->cnt == 2);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[1] == child);
    assert_true(child->val == L'z');
    trie_done(node);
}

static void trie_get_child_or_add_2A_test(void **state)
{
    struct trie_node *node = *state;
    struct trie_node *c1 = node->chd[0], *c2 = node->chd[1];
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
    struct trie_node *node = *state;
    struct trie_node *c1 = node->chd[0], *c2 = node->chd[1];
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
    struct trie_node *node = *state;
    struct trie_node *c1 = node->chd[0], *c2 = node->chd[1];
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
    struct trie_node *node = *state;
    struct trie_node *c1 = node->chd[0], *c2 = node->chd[1];
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
    struct trie_node *node = *state;
    struct trie_node *c1 = node->chd[0], *c2 = node->chd[1];
    struct trie_node *child = trie_get_child_or_add_empty(node, L'z');
    assert_true(node->cnt == 3);
    assert_true(node->cap >= node->cnt);
    assert_true(node->chd[0] == c1);
    assert_true(node->chd[1] == c2);
    assert_true(node->chd[2] == child);
    assert_true(child->val == L'z');
    trie_done(node);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(trie_get_child_index_empty_test, node_0_setup, node_0_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_index_1_test, node_1_setup, node_1_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_index_2_test, node_2_setup, node_2_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_index_4_test, node_4_setup, node_4_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_empty_test, node_0_setup, node_0_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_1_test, node_1_setup, node_1_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_2_test, node_2_setup, node_2_teardown),
        cmocka_unit_test_setup_teardown(trie_get_child_or_add_empty_test, node_0_setup, NULL),
        cmocka_unit_test_setup_teardown(trie_get_child_or_add_1A_test, node_1_setup, NULL),
        cmocka_unit_test_setup_teardown(trie_get_child_or_add_1B_test, node_1_setup, NULL),
        cmocka_unit_test_setup_teardown(trie_get_child_or_add_1C_test, node_1_setup, NULL),
        cmocka_unit_test_setup_teardown(trie_get_child_or_add_2A_test, node_2_setup, NULL),
        cmocka_unit_test_setup_teardown(trie_get_child_or_add_2B_test, node_2_setup, NULL),
        cmocka_unit_test_setup_teardown(trie_get_child_or_add_2C_test, node_2_setup, NULL),
        cmocka_unit_test_setup_teardown(trie_get_child_or_add_2D_test, node_2_setup, NULL),
        cmocka_unit_test_setup_teardown(trie_get_child_or_add_2E_test, node_2_setup, NULL),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
