/** @file
  Implementacja drzewa TRIE.

  @ingroup dictionary
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-05-24
 */

#include "trie.h"
#include "charmap.h"
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
static int trie_node_integrity(struct trie_node *node)
{
    if(node == NULL) return 0;
    if(node->cnt > node->cap) return 0;
    if(node->cap > 0 && node->chd == NULL) return 0;
    if(node->cap == 0 && node->chd != NULL) return 0;
    for(int i = 0; i < node->cnt; i++)
    {
        for(int j = 0; j < node->cnt; j++)
        {
            if(i != j && node->chd[i] == node->chd[j]) return 0;
        }
    }
    return 1;
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
static int trie_get_child_index(struct trie_node *node, wchar_t value, int begin, int end)
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
static struct trie_node * trie_get_child(struct trie_node *node, wchar_t value)
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
    struct trie_node *child = trie_get_child(node, word[0]);
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
 * Wypełnia char-mapę istniejącymi literami w słowniku.
 * 
 * Wypełnianie zostaje przerwane, gdy char-mapa zostanie wypełniona w całości.
 * Każdemu znakowi zostaje przypisana wartość 8-bitowa.
 * Dodatkowo zostaje obliczona długość najdłuższego słowa.
 * 
 * @param[in] node Drzewo, którego litery wrzucić do char-mapy.
 * @param[in,out] map Char-mapa do wypełnienia.
 * @param[in,out] trans Przyporządkowanie wartości 8-bitowej do znaku ze słownika.
 * @param[in,out] symbol Kolejna wartość 8-bitowa do przypisania.
 * @param[in] length Głębokość w drzewie, na której aktualnie jesteśmy.
 * @param[in,out] maxlength Największa do tej pory uzyskana głębokość.
 */
static void trie_fill_charmap(struct trie_node *node, struct char_map *map, wchar_t **trans, char *symbol, int length, int *maxlength)
{
    assert(trie_node_integrity(node));
    if(length > *maxlength) *maxlength = length;
    if(char_map_size(map) < char_map_capacity())
    {
        if(char_map_put(map, node->val, *symbol))
        {
            (*symbol)++;
            (**trans) = node->val;
            (*trans)++;
        }
    }
    for(int i = 0; i < node->cnt; i++)
    {
        trie_fill_charmap(node->chd[i], map, trans, symbol, length + 1, maxlength);       
    }
}

/**
 * Wypisuje do pliku instrukcje odpowiadające za skakanie w górę drzewa.
 * 
 * @param[in] res Ilość skoków w górę drzewa.
 * @param[in] count Ilość liter w alfabecie.
 * @param[in] loglen Sufit z logarytmu z maksymalnej długości słowa.
 * @param[in] file Plik, do którego zapisać wygenerowane instrukcje.
 */
static void trie_serialize_formatA_ender(int res, int count, int loglen, FILE *file)
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
 * Wypisuje do pliku instrukcje odpowiadające za reprezentację danego poddrzewa.
 * 
 * @param[in] node Poddrzewo, które mamy reprezentować.
 * @param[in] map Char-mapa przypisująca znakowi wartość 8-bitową.
 * @param[in] count Ilość liter w alfabecie.
 * @param[in] loglen Sufit z logarytmu z maksymalnej długości słowa.
 * @param[in] file Plik, do którego zapisać wygenerowane instrukcje.
 * 
 * @return Ilość skoków w górę, którą należy jeszcze zapisać do pliku.
 */
static int trie_serialize_formatA_helper(struct trie_node *node, struct char_map *map, int count, int loglen, FILE *file)
{
    assert(trie_node_integrity(node));
    char coded = 0;
    assert(char_map_get(map, node->val, &coded));
    fputc(coded, file);
    if(node->cnt == 0) return 1;
    else if(node->leaf) fputc((char)count, file);
    for(int i = 0; i + 1 < node->cnt; i++)
    {
        int res = trie_serialize_formatA_helper(node->chd[i], map, count, loglen, file);
        // write way up...
        trie_serialize_formatA_ender(res, count, loglen, file);
    }
    int res = trie_serialize_formatA_helper(node->chd[node->cnt-1], map, count, loglen, file);
    return res + 1;
}


/**
 * Wypisuje do pliku instrukcje odpowiadające za reprezentację danego drzewa.
 * 
 * Drzewo zostanie zapisane w formacie "dictA".
 * 
 * @param[in] root Drzewo, które mamy reprezentować.
 * @param[in] map Char-mapa przypisująca znakowi wartość 8-bitową.
 * @param[in] trans Przypisanie 8-bitowej wartości znakowi.
 * @param[in] length Maksymalna długość słowa w słowniku.
 * @param[in] loglen Sufit z logarytmu z length
 * @param[in] file Plik, do którego zapisać wygenerowane instrukcje.
 */
static void trie_serialize_formatA(struct trie_node *root, struct char_map *map, wchar_t *trans, int length, int loglen, FILE *file)
{
    assert(trie_node_integrity(root));
    // Nagłówek pliku
    fputs("dictA", file);
    int count = char_map_size(map);
    fputc((char)count, file);
    fputc((char)loglen, file);
    
    
    // Wygenerowanie danych przypisujących liczbę 8-bitową znakowi.
    char * trans_buffer = malloc(sizeof(char)*256*16);
    int tb_len = 0;
    int tb_cap = 256*16;
    char mb[MB_CUR_MAX];
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    for(int i = 0; i < char_map_size(map); i++)
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
    // Długość danych przypisujących wartość 8-bitową znakowi
    fputc((char)((tb_len>>24)&0xFF), file);
    fputc((char)((tb_len>>16)&0xFF), file);
    fputc((char)((tb_len>>8)&0xFF), file);
    fputc((char)(tb_len&0xFF), file);
    
    // Dane przypisujące wartość 8-bitową znakowi
    for(int i = 0; i < tb_len; i++)
    {
        fputc(trans_buffer[i], file);
    }
    free(trans_buffer);
    
    // Znaczenie znaku w zapisie drzewa...
    // 0 .. (count-1) = characters
    // count = end of word
    // (count+1) .. (count+loglen+1) = end of word and go up
    // (count+loglen+2) .. = other end of word and go up
    
    // Zapisanie drzewa
    for(int i = 0; i < root->cnt; i++)
    {
        int res = trie_serialize_formatA_helper(root->chd[i], map, count, loglen, file);
        // write way up...
        trie_serialize_formatA_ender(res, count, loglen, file);
    }
    // Koniec drzewa.
    fputc((char)(count+1), file);
}

/**
 * Wczytuje poddrzewo z pliku.
 * 
 * @param[in,out] node Korzeń podderzewa do wczytania.
 * @param[in] lcount Ilość liter w alfabecie.
 * @param[in] loglen Sufit z logarytmu z maksymalnej długości słowa.
 * @param[in] translator Przypisanie 8-bitowej wartości znakowi.
 * @param[in] file Strumień, z którego wczytać poddrzewo.
 * 
 * @return Ilość skoków do zrobienia w górę.
 */
static int trie_deserialize_formatA_helper(struct trie_node *node, int lcount, int loglen, wchar_t * translator, FILE *file)
{
    assert(trie_node_integrity(node));
    // Czy instrukcja skoku w górę powinna oznaczać węzeł jako liść.
    int setleaf = 1;
    while(1)
    {
        int cmd = fgetc(file);
        if(cmd < 0) assert(0);  // WRONG FILE?
        // Obsługa różnych rodzajów instrukcji
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
            assert(trie_node_integrity(node));
            return (1<<(cmd - lcount - 1))-1;
        }
        else
        {
            if(setleaf) node->leaf = 1;
            assert(trie_node_integrity(node));
            return cmd - lcount - loglen - 1;
        }
    }
}


/**
 * Wczytuje drzewo z pliku w formacie "dictA"
 * 
 * @param[in] file Strumień, z którego wczytać drzewo.
 * @return Wskaźnik na wczytane drzewo.
 */
static struct trie_node * trie_deserialize_formatA(FILE *file)
{
    // Nagłówki
    int lcount = fgetc(file);
    int loglen = fgetc(file);
    int tb_len = fgetc(file)&0xFF;
    tb_len = (tb_len << 8) | (fgetc(file)&0xFF);
    tb_len = (tb_len << 8) | (fgetc(file)&0xFF);
    tb_len = (tb_len << 8) | (fgetc(file)&0xFF);
    
    // Przyporządkowanie 8-bitowej wartości znakowi.
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
    
    // Wczytanie drzewa.
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
        memcpy(newstr, *string, length);
        free(*string);
        *string = newstr;
    }
}

/**
 * Znajduje podpowiedzi w danym poddrzewie.
 * 
 * @param[in] node Poddrzewo do przeszukania.
 * @param[in] word Podsłowo wzorcowe, wg którego szukać podpowiedzi.
 * @param[in,out] created Słowo reprezentowane przez node'a.
 * @param[in] length Długość słowa created.
 * @param[in,out] capacity Wskaźnik na pojemność stringu created.
 * @param[in,out] points Ilość dozwolonych jeszcze zmian w podsłowie wzorcowym.
 * @param[in,out] list Lista wygenerowanych podpowiedzi.
 */
static void trie_hints_helper(struct trie_node *node, const wchar_t *word,
                       wchar_t **created, int length, int *capacity,
                       int points, struct word_list *list)
{
    assert(trie_node_integrity(node));
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

/**
 * Porównuje dwa stringi alfabetycznie.
 * 
 * Komparator dla qsort.
 * 
 * @param[in] a Wskaźnik na pierwszego stringa.
 * @param[in] b Wskaźnik na drugiego stringa.
 * 
 * @return Wynik porównania.
 */
static int locale_sorter(const void *a, const void *b)
{
    return wcscoll(*(const wchar_t**)a, *(const wchar_t**)b);
}

/**
 * Wypisuje słowa z danego poddrzewa.
 * 
 * @param[in] node Węzęł reprezentujący poddrzewo.
 * @param[in] str Prefiks dodany do słowa
 * @param[in] len Długość prefiksu.
 * @param[in,out] cap Wskaźnik na miejsce w pamięci gdzie przechowywana jest pojemność str.
 */
void trie_print_helper(struct trie_node *node, wchar_t **str, int len, int *cap)
{
    assert(trie_node_integrity(node));
    fix_size(str, len + 2, cap);
    (*str)[len] = node->val;
    if(node->leaf)
    {
	(*str)[len+1] = L'\0';
	printf("%ls\n", *str);
    }
    for(int i = 0; i < node->cnt; i++)
    {
	trie_print_helper(node->chd[i], str, len+1, cap);
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

int trie_find(struct trie_node* root, const wchar_t* word)
{
    assert(trie_node_integrity(root));
    if(word[0] == 0) return root->leaf;
    struct trie_node *child = trie_get_child(root, word[0]);
    if(child == NULL) return 0;
    return trie_find(child, word + 1);
}

int trie_delete(struct trie_node* root, const wchar_t* word)
{
    assert(trie_node_integrity(root));
    assert(word[0] != 0);
    struct trie_node *child = trie_get_child(root, word[0]);
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

void trie_serialize(struct trie_node *root, FILE *file)
{
    assert(trie_node_integrity(root));
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
    // we need at least 1 + loglen special characters!
    int loglen = 0;
    while((length+1) > (1<<loglen)) loglen++;
    if(char_map_size(map) <= 255-loglen)
    {
        trie_serialize_formatA(root, map, trans, length, loglen, file);
    }
    else
    {
        assert(0 && "Unimplemented");
    }
    char_map_done(map);
}


struct trie_node * trie_deserialize(FILE *file)
{
    char header[6];
    if(fgets(header, 6, file) == NULL) return NULL;
    if(strncmp(header, "dict", 4) != 0) return NULL;
    struct trie_node *ret = NULL;
    switch(header[4])
    {
        case 'A': ret = trie_deserialize_formatA(file); break;
        default: ret = NULL;
    }
    assert(trie_node_integrity(ret));
    return ret;
}

void trie_hints(struct trie_node *root, const wchar_t *word, struct word_list *list)
{
    assert(trie_node_integrity(root));
    struct word_list mylist;
    word_list_init(&mylist);
    int capacity = 1024;
    wchar_t *buff = malloc(sizeof(wchar_t)*capacity);
    trie_hints_helper(root, word, &buff, 0, &capacity, 1, &mylist);
    free(buff);
    const wchar_t ** array = (const wchar_t **)word_list_get(&mylist);
    qsort(array, word_list_size(&mylist), sizeof(wchar_t*), locale_sorter);
    if(word_list_size(&mylist) > 0) word_list_add(list, array[0]);
    for(int i = 1; i < word_list_size(&mylist); i++)
    {
        if(wcscmp(array[i-1], array[i]) != 0)
        {
            word_list_add(list, array[i]);
        }
    }
    word_list_done(&mylist);
}

void trie_print(struct trie_node *root)
{
    assert(trie_node_integrity(root));
    int cap = 1024;
    wchar_t *str = malloc(sizeof(wchar_t)*cap);
    for(int i = 0; i < root->cnt; i++)
    {
        trie_print_helper(root->chd[i], &str, 0, &cap);
    }
    free(str);
}

/**@}*/




