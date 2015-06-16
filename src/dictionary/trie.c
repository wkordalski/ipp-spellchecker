/** @file
  Implementacja drzewa TRIE.

  @ingroup dictionary
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-06-03
 */

#include "trie.h"

#include "list.h"
#include "rule.h"
#include "word_list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

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


/** @name Funkcje pomocnicze
 * @{
 */

/**
 * Sprawdza niektóre niezmienniki dla węzła.
 * 
 * @param[in] node Węzeł do przetestowania.
 * 
 * @return 1 jeśli węzeł jest poprawny, 0 otherwise.
 */
static int trie_node_integrity(const struct trie_node *node)
{
    if(node == NULL) goto fail;
    if(node->cnt > node->cap) goto fail;
    if(node->cap > 0 && node->chd == NULL) goto fail;
    if(node->cap == 0 && node->chd != NULL) goto fail;
    for(int i = 0; i < node->cnt; i++)
    {
        for(int j = 0; j < node->cnt; j++)
        {
            if(i != j && node->chd[i] == node->chd[j]) goto fail;
        }
    }
    return 1;
fail:
    return 0;
}

/**
 * Znajduje gdzie powinien być node o wartości value wśród dzieci pewnego węzła.
 * 
 * @param[in] node Węzeł, którego dzieci powinny zostać przeszukane.
 * @param[in] value Wartość węzła do znalezienia.
 * @param[in] begin Początek przedziału, w którym może się znaleźć szukany węzeł.
 * @param[in] end Koniec przedziału, w którym może się znaleźć szukany węzeł.
 * 
 * @return -1 jeśli trzeba utworzyć listę dzieci lub indeks, na którym powinien
 * być dany element (trzeba sprawdzić czy rzeczywiście tam jest)
 */
static int trie_get_child_index(const struct trie_node *node, wchar_t value, int begin, int end)
{
    assert(trie_node_integrity(node));
    assert(0 <= begin && begin <= end && end <= node->cnt);
    if(node->chd == NULL) return -1;
    while(end - begin > 2)
    {
        int middle = (begin + end)/2;
        wchar_t midval = node->chd[middle]->val;
        if(midval == value)
            return middle;
        else if(midval > value)
            end = middle;
        else
            begin = middle + 1;
    }
    for(int i = begin; i < end; i++)
    {
        if(node->chd[i]->val >= value) return i;
    }
    return end;
}

/**
 * Zwraca dziecko węzła o podanej wartości.
 * 
 * @param[in] node Węzeł, którego dzieci przeszukać.
 * @param[in] value Wartość, którą znaleźć.
 * 
 * @return Wskaźnik na znaleziony węzeł lub NULL jeśli nie znaleziono.
 */
static struct trie_node * trie_get_child_priv(struct trie_node *node, wchar_t value)
{
    assert(trie_node_integrity(node));
    int r = trie_get_child_index(node, value, 0, node->cnt);
    if(r == -1) return NULL;
    if(r == node->cnt) return NULL;
    if(node->chd[r]->val != value) return NULL;
    return node->chd[r];
}

/**
 * Zwraca dziecko węzła o podanej wartości lub tworzy takowe dziecko.
 * 
 * @param[in,out] node Węzeł, którego dzieci przeszukać.
 * @param[in] value Wartość którą znaleźć.
 * 
 * @return Wskaźnik na znaleziony lub utworzony węzeł.
 */
static struct trie_node * trie_get_child_or_add_empty(struct trie_node *node, wchar_t value)
{
    int r = trie_get_child_index(node, value, 0, node->cnt);
    if(r == -1)
    {
        // Let's add an empty children list
        node->chd = malloc(4 * sizeof(struct trie_node *));
        node->cnt = 1;
        node->cap = 4;
        node->chd[0] = trie_init();
        node->chd[0]->val = value;
        return node->chd[0];
    }
    else if(r < node->cnt && node->chd[r]->val == value)
    {
        return node->chd[r];
    }
    else
    {
        if(node->cnt < node->cap)
        {
            for(int i = node->cnt - 1; i >= r; i--)
            {
                node->chd[i + 1] = node->chd[i];
            }
        }
        else
        {
            node->cap *= 2;
            struct trie_node ** table = malloc((node->cap) * sizeof(struct trie_node *));
            struct trie_node ** source = node->chd;
            for(int i = 0; i < r; i++)
            {
                table[i] = source[i];
            }
            for(int i = r; i < node->cnt; i++)
            {
                table[i + 1] = source[i];
            }
            node->chd = table;
            free(source);
        }
        node->cnt++;
        node->chd[r] = trie_init();
        node->chd[r]->val = value;
        return node->chd[r];
    }
}


/**
 * Gdy można usuwa node i aktualizuje parenta.
 * 
 * @param[in] node Węzeł, który ewentualnie usunąć.
 * @param[in,out] parent Rodzic node'a.
 */
static void trie_cleanup(struct trie_node *node, struct trie_node *parent)
{
    if(node->leaf != 0 || node->cnt > 0) return;
    int r = trie_get_child_index(parent, node->val, 0, parent->cnt);
    assert(r >= 0 && r < parent->cnt && parent->chd[r] == node);
    if(parent->cnt == 1)
    {
        parent->cnt = 0;
        parent->cap = 0;
        free(parent->chd);
        parent->chd = NULL;
    }
    else
    {
        assert(parent->cnt > 1);
        struct trie_node **table;
        struct trie_node **source = parent->chd;
        if(parent->cnt * 3 < parent->cap && parent->cap > 4)
        {
            parent->cap /= 2;
            table = malloc((parent->cap)*sizeof(struct trie_node*));
            for(int i = 0; i < r; i++)
            {
                table[i] = source[i];
            }
            for(int i = r + 1; i < parent->cnt; i++)
            {
                table[i-1] = source[i];
            }
            parent->chd = table;
            parent->cnt--;
            free(source);
        }
        else
        {
            table = parent->chd;
            for(int i = r + 1; i < parent->cnt; i++)
            {
                table[i-1] = source[i];
            }
            parent->cnt--;
        }

    }
    free(node);
}

/**
 * Funkcja usuwająca podsłowo z poddrzewa.
 * 
 * @param[in,out] node Poddrzewo, z którego usunąć podsłowo.
 * @param[in,out] parent Rodzic node'a.
 * @param[in] word Podsłowo do usunięcia.
 * 
 * @return Zwraca 1 jeśli podsłowo zostało usunięte, 0 jeśli nie istniało.
 */
static int trie_delete_helper(struct trie_node *node, struct trie_node *parent, const wchar_t *word)
{
    assert(trie_node_integrity(node));
    if(word[0] == 0)
    {
        if(node->leaf)
        {
            node->leaf = 0;
            trie_cleanup(node, parent);
            assert(trie_node_integrity(parent));
            return 1;
        }
        else
        {
            return 0;
        }
    }
    struct trie_node *child = trie_get_child_priv(node, word[0]);
    if(child == NULL)
    {
        return 0;
    }
    else
    {
        int r = trie_delete_helper(child, node, word + 1);
        trie_cleanup(node, parent);
        assert(trie_node_integrity(parent));
        return r;
    }
}

/**
 * Wypisuje do pliku instrukcje odpowiadające za reprezentację danego poddrzewa.
 * 
 * @param[in] node Poddrzewo, które mamy reprezentować.
 * @param[in] file Plik, do którego zapisać wygenerowane instrukcje.
 * 
 * @return 0 jeśli zapisano z sukcesem, -1 w p.p.
 */
static int trie_serialize_formatU_helper(struct trie_node *node, FILE *file)
{
    assert(trie_node_integrity(node));
    
    if(fputwc(node->val, file)<0) return -1;
    if(node->leaf)
        if(fputwc(1, file)<0) return -1;
    for(int i = 0; i < node->cnt; i++)
        if(trie_serialize_formatU_helper(node->chd[i], file)<0) return -1;
    if(fputwc(2, file)<0) return -1;
    return 0;
}

/**
 * Wypisuje do pliku instrukcje odpowiadające za reprezentację danego drzewa.
 * 
 * Drzewo zostanie zapisane w formacie "dictA".
 * 
 * @param[in] root Drzewo, które mamy reprezentować.
 * @param[in] file Plik, do którego zapisać wygenerowane instrukcje.
 */
static int trie_serialize_formatU(struct trie_node *node, FILE *file)
{
    for(int i = 0; i < node->cnt; i++)
        if(trie_serialize_formatU_helper(node->chd[i], file)<0) return -1;
    if(fputwc(2, file)<0) return -1;
    return 0;
}

/**
 * Wczytuje poddrzewo z pliku.
 * 
 * @param[in,out] node Korzeń podderzewa do wczytania.
 * @param[in] file Strumień, z którego wczytać poddrzewo.
 * 
 * @return -1 jeśli błąd, 0 jeśli OK
 */
static int trie_deserialize_formatU_helper(struct trie_node *node, FILE *file)
{
    assert(trie_node_integrity(node));
    while(1)
    {
        wchar_t cmd = fgetwc(file);
        if(cmd <= 0) return -1;
        // Obsługa różnych rodzajów instrukcji
        if(cmd == 1) node->leaf = 1;
        else if(cmd == 2) break;
        else
        {
            // add letter
            struct trie_node * child = trie_get_child_or_add_empty(node, cmd);
            if(trie_deserialize_formatU_helper(child, file)<0) return -1;
        }
    }
    assert(trie_node_integrity(node));
    return 0;
}

/**
 * Wczytuje drzewo z pliku.
 * 
 * @param[in] file Strumień, z którego wczytać poddrzewo.
 * 
 * @return Wczytane drzewo lub NULL jeśli błąd.
 */
static struct trie_node * trie_deserialize_formatU(FILE *file)
{
    struct trie_node *root = trie_init();
    while(1)
    {
        wchar_t cmd = fgetwc(file);
        if(cmd <= 1)
        {
            trie_done(root);
            return NULL;
        }
        else if(cmd == 2) break;
        else
        {
            // add letter
            struct trie_node * child = trie_get_child_or_add_empty(root, cmd);
            if(trie_deserialize_formatU_helper(child, file)<0)
            {
                trie_done(root);
                return NULL;
            }
        }
    }
    assert(trie_node_integrity(root));
    return root;
}

/**
 * Wydłuża string-a jeśli trzeba, aby móc dodać do niego kolejną literę.
 * 
 * @param[in,out] string Wskaźnik na stringa, którego ew. wydłużyć.
 * @param[in] length Aktualna długość stringa.
 * @param[in,out] capacity Aktualna pojemność stringa.
 */
static void fix_size(wchar_t **string, int length, int *capacity)
{
    if(length >= *capacity)
    {
        (*capacity) *= 2;
        wchar_t * newstr = malloc(sizeof(wchar_t)*(*capacity));
        memcpy(newstr, *string, (*capacity)/2);
        free(*string);
        *string = newstr;
    }
}

/**
 * @}
 */

/** @name Elementy interfejsu 
   @{
 */

struct trie_node * trie_init()
{
    struct trie_node *root = malloc(sizeof(struct trie_node));
    root->val = 0;
    root->cnt = 0;
    root->leaf = 0;
    root->cap = 0;
    root->chd = NULL;
    assert(trie_node_integrity(root));
    return root;
}

void trie_done(struct trie_node *root)
{
    assert(trie_node_integrity(root));
    trie_clear(root);
    free(root);
}


void trie_clear(struct trie_node *node)
{
    assert(trie_node_integrity(node));
    if(node->chd == NULL) return;
    for(int i = 0; i < node->cnt; i++)
    {
        trie_clear(node->chd[i]);
        free(node->chd[i]);
    }
    free(node->chd);
    node->chd = NULL;
    node->cap = 0;
    node->cnt = 0;
    assert(trie_node_integrity(node));
}

int trie_insert(struct trie_node* root, const wchar_t* word)
{
    assert(trie_node_integrity(root));
    assert(word[0] != 0);
    struct trie_node *child = trie_get_child_or_add_empty(root, word[0]);
    if(word[1] == 0)
    {
        // Trzeba sprawdzić, czy słowo przypadkiem już nie istnieje!
        if(child->leaf) return 0;
        child->leaf = 1;
        assert(trie_node_integrity(root));
        return 1;
    }
    else
    {
        // Zwyczajnie dodajemy
        int r = trie_insert(child, word + 1);
        assert(trie_node_integrity(root));
        return r;
    }
}

int trie_find(const struct trie_node* root, const wchar_t* word)
{
    assert(trie_node_integrity(root));
    if(word[0] == 0) return root->leaf;
    const struct trie_node *child = trie_get_child(root, word[0]);
    if(child == NULL) return 0;
    return trie_find(child, word + 1);
}

int trie_delete(struct trie_node* root, const wchar_t* word)
{
    assert(trie_node_integrity(root));
    assert(word[0] != 0);
    struct trie_node *child = trie_get_child_priv(root, word[0]);
    if(child == NULL)
    {
        return 0;
    }
    else
    {
        int r = trie_delete_helper(child, root, word + 1);
        assert(trie_node_integrity(root));
        return r;
    }
}

int trie_serialize(struct trie_node *root, FILE *file)
{
    assert(trie_node_integrity(root));
    return trie_serialize_formatU(root, file);
}


struct trie_node * trie_deserialize(FILE *file)
{
    struct trie_node *ret = trie_deserialize_formatU(file);
    assert(trie_node_integrity(ret));
    return ret;
}

const struct trie_node * trie_get_child(const struct trie_node *node, wchar_t value)
{
    assert(trie_node_integrity(node));
    int r = trie_get_child_index(node, value, 0, node->cnt);
    if(r == -1) return NULL;
    if(r == node->cnt) return NULL;
    if(node->chd[r]->val != value) return NULL;
    return node->chd[r];
}

int trie_get_children(const struct trie_node *node, const struct trie_node *** children)
{
    (*children) = (const struct trie_node**)node->chd;
    return node->cnt;
}

wchar_t trie_get_value(const struct trie_node *node)
{
    return node->val;
}

const wchar_t * trie_get_value_ptr(const struct trie_node *node)
{
    return &(node->val);
}

bool trie_is_leaf(const struct trie_node *node)
{
    return node->leaf;
}

void trie_hints(struct trie_node *root, const wchar_t *word, struct word_list *list, struct list *rules, int max_cost, int max_hints_no)
{
    assert(trie_node_integrity(root));
    list_terminate(rules);
    struct list *output = rule_generate_hints((struct hint_rule**)list_get(rules), max_cost, max_hints_no, root, word);
    for(int i = 0; i < list_size(output); i++)
    {
        word_list_add(list, list_get(output)[i]);
        free(list_get(output)[i]);
    }
}


/**@}*/




