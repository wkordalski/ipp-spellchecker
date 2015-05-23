#include "trie.h"
#include "charmap.h"
#include "word_list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

struct trie_node
{
    wchar_t val;                ///< Wartość węzła
    unsigned int cap : 32;      ///< Pojemność tablicy dzieci
    unsigned int cnt : 31;      ///< Ilość dzieci
    unsigned int leaf : 1;      ///< Czy tutaj kończy się słowo
    struct trie_node **chd;     ///< Lista dzieci
};

struct trie_node * trie_init()
{
    struct trie_node *root = malloc(sizeof(struct trie_node));
    root->val = 0;
    root->cnt = 0;
    root->leaf = 0;
    root->cap = 0;
    root->chd = NULL;
    return root;
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
    assert(node != NULL);
    int r = trie_get_child_index(node, value, 0, node->cnt);
    if(r == -1) return NULL;
    if(r == node->cnt) return NULL;
    if(node->chd[r]->val != value) return NULL;
    return node->chd[r];
}

struct trie_node * trie_get_child_or_add_empty(struct trie_node *node, wchar_t value)
{
    int r = trie_get_child_index(node, value, 0, node->cnt);
    if(r == -1)
    {
        // Let's add an empty children list
        node->chd = malloc(4 * sizeof(struct node *));
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
            struct trie_node ** table = malloc((node->cnt+1) * sizeof(struct node *));\
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
int trie_insert(struct trie_node* root, const wchar_t* word)
{
    assert(word[0] != 0);
    struct trie_node *child = trie_get_child_or_add_empty(root, word[0]);
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
int trie_find(struct trie_node* root, const wchar_t* word)
{
    if(word[0] == 0) return root->leaf;
    struct trie_node *child = trie_get_child(root, word[0]);
    if(child == NULL) return 0;
    return trie_find(child, word + 1);
}

void trie_cleanup(struct trie_node *node, struct trie_node *parent)
{
    if(node->leaf != 0 || node->cnt > 0) return;
    int r = trie_get_child_index(parent, node->val, 0, node->cnt);
    assert(r != -1 && r < parent->cnt && parent->chd[r] == node);
    if(parent->cnt == 1)
    {
        parent->cnt = 0;
        parent->cap = 0;
        free(parent->chd);
        parent->chd = NULL;
    }
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
int trie_delete_helper(struct trie_node *node, struct trie_node *parent, const wchar_t *word)
{
    if(word[0] == 0)
    {
        if(node->leaf)
        {
            node->leaf = 0;
            trie_cleanup(node, parent);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    struct trie_node *child = trie_get_child(node, word[0]);
    if(child == NULL)
    {
      return 0;
    }
    else
    {
      int r = trie_delete_helper(child, node, word + 1);
      trie_cleanup(node, parent);
      return r;
    }
}

/**
 * @return 1 jeśli słowo zostało usunięte, 0 jeśli nie
 */
int trie_delete(struct trie_node* root, const wchar_t* word)
{
    assert(word[0] != 0);
    struct trie_node *child = trie_get_child(root, word[0]);
    if(child == NULL)
    {
      return 0;
    }
    else
    {
      return trie_delete_helper(child, root, word + 1);
    }
}


/**
 * @returns 0 if overflow of map encountred, 1 otherwise
 */
int trie_fill_charmap(struct trie_node *root, struct char_map *map, wchar_t **trans, char *symbol, int length, int *maxlength)
{
    if(length > *maxlength) *maxlength = length;
    if(char_map_count(map) >= char_map_capacity()) return 0;
    if(char_map_put(map, root->val, *symbol))
    {
        (*symbol)++;
        (**trans) = root->val;
        (*trans)++;
    }
    for(int i = 0; i < root->cnt; i++)
    {
        if(!trie_fill_charmap(root->chd[i], map, trans, symbol, length + 1, maxlength)) return 0;       
    }
    return 1;
}

void trie_serialize_formatA_ender(int res, int count, int loglen, FILE *file)
{
    if(res < 256 - (count+loglen))
    {
        fputc((char)(count+loglen+res), file);
    }
    else
    {
        for(int j = 0; res > (1<<j) && j < loglen; j++)
        {
            if(res & (1<<j)) fputc((char)(count+1+j), file);
        }
    }
}

/**
 * @returns number of hops to do up
 */
int trie_serialize_formatA_helper(struct trie_node *root, struct char_map *map, int count, int loglen, FILE *file)
{
    char coded = 0;
    assert(char_map_get(map, root->val, &coded));
    fputc(coded, file);
    if(root->cnt == 0) return 1;
    else if(root->leaf) fputc((char)count, file);
    for(int i = 0; i + 1 < root->cnt; i++)
    {
        int res = trie_serialize_formatA_helper(root->chd[i], map, count, loglen, file);
        // write way up...
        trie_serialize_formatA_ender(res, count, loglen, file);
    }
    int res = trie_serialize_formatA_helper(root->chd[root->cnt-1], map, count, loglen, file);
    return res + 1;
}

void trie_serialize_formatA(struct trie_node *root, struct char_map *map, wchar_t *trans, int length, FILE *file)
{
    fputs("dictA", file);
    int loglen = 0;
    while((length+1) > (1<<loglen)) loglen++;
    int count = char_map_count(map);
    fputc((char)count, file);
    fputc((char)loglen, file);
    char * trans_buffer = malloc(sizeof(char)*256*16);
    int tb_len = 0;
    int tb_cap = 256*16;
    char mb[MB_CUR_MAX];
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    for(int i = 0; i < char_map_count(map); i++)
    {
        int len = wcrtomb(mb, trans[i], &state);
        for(int j = 0; j < len; j++)
        {
            trans_buffer[tb_len++] = mb[j];
            if(tb_len >= tb_cap)
            {
                tb_cap *= 2;
                char *tb2 = malloc(sizeof(char)*tb_cap);
                memcpy(tb2, trans_buffer, tb_len*sizeof(char));
                free(trans_buffer);
                trans_buffer = tb2;
            }
        }
    }
    fputc((char)((tb_len>>24)&0xFF), file);
    fputc((char)((tb_len>>16)&0xFF), file);
    fputc((char)((tb_len>>8)&0xFF), file);
    fputc((char)(tb_len&0xFF), file);
    
    for(int i = 0; i < tb_len; i++)
    {
        fputc(trans_buffer[i], file);
    }
    free(trans_buffer);
    
    // 0 .. (count-1) = characters
    // count = end of word
    // (count+1) .. (count+loglen+1) = end of word and go up
    // (count+loglen+2) .. = other end of word and go up
    for(int i = 0; i < root->cnt; i++)
    {
        int res = trie_serialize_formatA_helper(root->chd[i], map, count, loglen, file);
        // write way up...
        trie_serialize_formatA_ender(res, count, loglen, file);
    }
    fputc((char)(count+1), file);
}

void trie_serialize(struct trie_node *root, FILE *file)
{
    struct char_map * map = char_map_init();
    char symbol = 0;
    int length = 0;
    wchar_t trans[256];
    wchar_t *tptr = trans;
    wchar_t **tend = &tptr;
    for(int i = 0; i < root->cnt; i++)
    {
        trie_fill_charmap(root->chd[i], map, tend, &symbol, 0, &length);
    }
    // we need at least 2 special characters!
    if(char_map_count(map) <= 254)
    {
        trie_serialize_formatA(root, map, trans, length, file);
    }
    else
    {
        assert(0 && "Unimplemented");
    }
    char_map_done(map);
}

int trie_deserialize_formatA_helper(struct trie_node *node, int lcount, int loglen, wchar_t * translator, FILE *file)
{
    int setleaf = 1;
    while(1)
    {
        int cmd = fgetc(file);
        if(cmd < 0) assert(0);  // WRONG FILE?
        else if(cmd < lcount)
        {
            // add letter
            struct trie_node * child = trie_get_child_or_add_empty(node, translator[cmd]);
            setleaf = 1;
            int ret = trie_deserialize_formatA_helper(child, lcount, loglen, translator, file);
            if(ret <= 0) setleaf = 0;
            if(ret > 0)  return ret - 1;
            continue;
        }
        else if(cmd == lcount)
        {
            node->leaf = 1;
            continue;
        }
        else if(cmd <= lcount + loglen)
        {
            if(setleaf) node->leaf = 1;
            return (1<<(cmd - lcount - 1))-1;
        }
        else
        {
            if(setleaf) node->leaf = 1;
            return cmd - lcount - loglen - 1;
        }
    }
}

struct trie_node * trie_deserialize_formatA(FILE *file)
{
    int lcount = fgetc(file);
    int loglen = fgetc(file);
    int tb_len = fgetc(file)&0xFF;
    tb_len = (tb_len << 8) | (fgetc(file)&0xFF);
    tb_len = (tb_len << 8) | (fgetc(file)&0xFF);
    tb_len = (tb_len << 8) | (fgetc(file)&0xFF);
    wchar_t *translator = malloc(sizeof(wchar_t)*lcount);
    {
        char *trans_map = malloc(sizeof(char)*(tb_len+1));
        char *tr_end = trans_map + tb_len;
        char *tr_cur = trans_map;
        if(fgets(trans_map, tb_len+1, file) == NULL) return NULL;
        mbstate_t state;
        memset(&state, 0, sizeof(state));
        for(int i = 0; i < lcount; i++)
        {
            int len = mbrtowc(translator + i, tr_cur, tr_end-tr_cur, &state);
            tr_cur += ((len>0)?len:1);
        }
        free(trans_map);
    }
    struct trie_node *root = trie_init();
    while(1)
    {
        int cmd = fgetc(file);
        if(cmd == EOF) break;
        if(cmd < 0) assert(0);
        if(cmd < lcount)
        {
            // add letter
            struct trie_node * child = trie_get_child_or_add_empty(root, translator[cmd]);
            int ret = trie_deserialize_formatA_helper(child, lcount, loglen, translator, file);
            if(ret == 0) continue;
            if(ret > 0) break;
        }
        if(cmd >= lcount) break;
    }
    free(translator);
    return root;
}

struct trie_node * trie_deserialize(FILE *file)
{
    char header[6];
    if(fgets(header, 6, file) == NULL) return NULL;
    if(strncmp(header, "dict", 4) != 0) return NULL;
    switch(header[4])
    {
        case 'A': return trie_deserialize_formatA(file);
        default: return NULL;
    }
}

void fix_size(wchar_t **created, int length, int *capacity)
{
    if(length >= *capacity)
    {
        (*capacity) *= 2;
        wchar_t * newstr = malloc(sizeof(wchar_t)*(*capacity));
        memcpy(newstr, *created, length);
        free(*created);
        *created = newstr;
    }
}

void trie_hints_helper(struct trie_node *node, const wchar_t *word,
                       wchar_t **created, int length, int *capacity,
                       int points, struct word_list *list)
{
    // We can only add one letter, so...
    fix_size(created, length + 1, capacity);
    if(points <= 0)
    {
        // No change
        if(word[0] == L'\0')
        {
            (*created)[length] = L'\0';
            if(node->leaf) word_list_add(list, *created);
        }
        else
        {
            struct trie_node *child = trie_get_child(node, word[0]);
            if(child != NULL)
            {
                (*created)[length] = word[0];
                trie_hints_helper(child, word+1,created, length+1, capacity, 0, list);
            }
        }
    }
    else
    {
        if(word[0] == L'\0')
        {
            // No change
            (*created)[length] = L'\0';
            if(node->leaf) word_list_add(list, *created);
            
            // Add some letter
            for(int i = 0; i < node->cnt; i++)
            {
                (*created)[length] = node->chd[i]->val;
                trie_hints_helper(node->chd[i], word, created, length+1, capacity, points-1, list);
            }
        }
        else
        {
            // No change
            struct trie_node *child = trie_get_child(node, word[0]);
            if(child != NULL)
            {
                (*created)[length] = word[0];
                trie_hints_helper(child, word+1,created, length+1, capacity, points, list);
            }
            // Miss the current letter
            trie_hints_helper(node, word+1, created, length, capacity, points-1, list);
            // Add some letter or swap current with some letter
            for(int i = 0; i < node->cnt; i++)
            {
                (*created)[length] = node->chd[i]->val;
                trie_hints_helper(node->chd[i], word, created, length+1, capacity, points-1, list);
                if(node->chd[i]->val != word[0])
                {
                    // But must be some change
                    trie_hints_helper(node->chd[i], word+1, created, length+1, capacity, points-1, list);
                }
            }
        }
    }
}

int locale_sorter(const void *a, const void *b)
{
    return wcscoll((const wchar_t*)a, (const wchar_t*)b);
}

void trie_hints(struct trie_node *root, const wchar_t *word, struct word_list *list)
{
    struct word_list mylist;
    word_list_init(&mylist);
    int capacity = 1024;
    wchar_t *buff = malloc(sizeof(wchar_t)*capacity);
    trie_hints_helper(root, word, &buff, 0, &capacity, 1, &mylist);
    free(buff);
    qsort(mylist.array, mylist.size, sizeof(wchar_t*), locale_sorter);
    word_list_add(list, mylist.array[0]);
    for(int i = 1; i < word_list_size(&mylist); i++)
    {
        if(wcscmp(mylist.array[i-1], mylist.array[i]) != 0)
        {
            word_list_add(list, mylist.array[1]);
        }
    }
    word_list_done(&mylist);
}