#include "trie.h"

#include <assert.h>
#include <stdlib.h>

struct trie_node * trie_init()
{
    struct trie_node *root = malloc(sizeof(struct trie_node));
    root->val = 0;
    root->cnt = 0;
    root->leaf = 0;
    root->chd = NULL;
}

void trie_done(struct trie_node *root)
{
    trie_clear(root);
    free(root);
}


/**
 * Znajduje gdzie powinien być node o wartości value.
 * 
 * @return -1 jeśli trzeba utworzyć listę dzieci
 * lub indeks na którym powinien być dany element
 * (trzeba sprawdzić czy rzeczywiście tam jest)
 */
int trie_get_child_index(struct trie_node *node, wchar_t value, int begin, int end)
{
    assert(node->chd != NULL || (node->chd == NULL && node->cnt == 0));
    if(node->chd == NULL) return -1;
    if(end - begin <= 2)
    {
        if(begin < end)
        {
            if(node->chd[begin]->val >= value) return begin;
            begin++;
        }
        if(begin < end)
        {
            if(node->chd[begin]->val >= value) return begin;
            begin++;
        }
        return begin;
    }
    else
    {
        int middle = (begin + end)/2;
        wchar_t midval = node->chd[middle]->val;
        if(midval == value)
            return middle;
        else if(midval > value)
            return trie_get_child_index(node, value, begin, middle);
        else
            return trie_get_child_index(node, value, middle+1, end);
    }
}

struct trie_node * trie_get_child(struct trie_node *node, wchar_t value)
{
    int r = trie_get_child_index(node, value, 0, node->cnt);
    if(r == -1) return NULL;
    if(node->chd[r]->val != value) return NULL;
    return node->chd[r];
}

struct trie_node * trie_get_child_or_add_empty(struct trie_node *node, wchar_t value)
{
    int r = trie_get_child_index(node, value, 0, node->cnt);
    if(r == -1)
    {
        // Let's add an empty children list
        node->chd = malloc(1 * sizeof(struct node *));
        node->cnt = 1;
        node->chd[0] = trie_init();
        return node->chd[0];
    }
    else
    {
        struct trie_node ** table = malloc((node->cnt+1) * sizeof(struct node *));\
        struct trie_node ** source = node->chd;
        for(int i = 0; i < r; i++)
        {
            table[i] = source[i];
        }
        table[r] = trie_init();
        for(int i = r; i < node->cnt; i++)
        {
            table[i + 1] = source[i];
        }
        node->cnt++;
        node->chd = table;
        free(source);
        return table[r];
    }
}

void trie_clear(struct trie_node *node)
{
    if(node->chd == NULL) return;
    for(int i = 0; i < node->cnt; i++)
    {
        trie_clear(node->chd[i]);
        free(node->chd[i]);
    }
    free(node->chd);
}


/**
 * @returns 1 jeśli słowo zostało dodane, 0 jeśli istniało wcześniej
 */
int trie_insert(struct trie_node *node, const wchar_t *word)
{
    assert(word[0] == 0);
    struct trie_node *child = trie_get_child_or_add_empty(node, word[0]);
    if(word[1] == 0)
    {
        // Trzeba sprawdzić, czy słowo przypadkiem już nie istnieje!
        if(child->leaf) return 0;
        child->leaf = 1;
        return 1;
    }
    else
    {
        // Chamsko dodajemy
        return trie_insert(child, word + 1);
    }
}

/**
 * @returns 1 jeśli słowo istnieje, 0 jeśli nie
 */
int trie_find(struct trie_node *node, const wchar_t *word)
{
    if(word[0] == 0) return node->leaf;
    struct trie_node *child = trie_get_child(node, word[0]);
    if(child == NULL) return 0;
    return trie_find(child, word + 1);
}

void trie_cleanup(struct trie_node *node, struct trie_node *parent)
{
    if(node->leaf == 1 || node->cnt > 0) return;
    int r = trie_get_child_index(parent, node->val);
    assert(r != -1 && parent->chd[r] == node);
    if(parent->cnt == 1) free(parent->chd);
    else
    {
        struct trie_node **table = malloc((parent->cnt - 1)*sizeof(struct trie_node*));
        struct trie_node **source = parent->chd;
        for(int i = 0; i < r; i++)
        {
            table[i] = source[i];
        }
        for(int i = r + 1; i < parent->cnt; i++)
        {
            table[i-1] = source[i];
        }
        parent->cnt--;
        parent->chd = table;
        free(source);
    }
    free(node);
}

/**
 * @return 1 jeśli słowo zostało usunięte, 0 jeśli nie
 */
int trie_delete(struct trie_node *node, const wchar_t *word)
{
    if(word[0] == 0)
    {
        // TODO
    }
    struct trie_node *child = trie_get_child(node, word[0]);
    //trie_delete()
    // TODO
}