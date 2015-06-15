/** @file
    Interfejs drzewa TRIE.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-15
 */

#ifndef __TRIE_H__
#define __TRIE_H__

/**
 * Węzeł drzewa TRIE.
 */
struct trie_node;


/*
 * Includes.
 */
#include "list.h"
#include "rule.h"
#include "word_list.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Tworzy nowy węzeł drzewa TRIE
 * 
 * @return Wskaźnik na drzewo TRIE.
 */
struct trie_node * trie_init();

/**
 * Destrukcja węzła drzewa TRIE wraz z poddrzewami.
 * 
 * @param[in] root Drzewo do usunięcia.
 */
void trie_done(struct trie_node *root);

/**
 * Usuwa wszystkie wyrazy z drzewa.
 * 
 * @param[in,out] root Drzewo do wyczyszczenia.
 */
void trie_clear(struct trie_node *root);

/**
 * Wstawia wyraz do drzewa.
 * 
 * @param[in,out] root Drzewo, do którego wstawić słowo.
 * @param[in] word Słowo do wstawienia.
 * @return 1 jeśli wstawiono słowo, 0 jeśli istniało wcześniej.
 */
int trie_insert(struct trie_node *root, const wchar_t *word);

/**
 * Sprawdza, czy słowo istnieje w drzewie.
 * 
 * @param[in] root Drzewo do przeszukania.
 * @param[in] word Słowo do znalezienia.
 * @return 0 jeśli nie znaleziono słowa, 1 gdy znaleziono.
 */
int trie_find(const struct trie_node *root, const wchar_t *word);

/**
 * Usuwa słowo z drzewa.
 * 
 * @param[in,out] root Drzewo, z którego usunąć słowo.
 * @param[in] word Słowo do usunięcia.
 * @return 1 jeśli usunięto słowo, 0 jeśli takowe słowo nie ustniało.
 */
int trie_delete(struct trie_node *root, const wchar_t *word);

/**
 * Zapisuje drzewo do strumienia.
 * 
 * @param[in] root Drzewo do zapisania.
 * @param[in] file Strumień, gdzie zapisać drzewo.
 */
int trie_serialize(struct trie_node *root, FILE *file);

/**
 * Ładuje drzewo ze strumienia.
 * 
 * @param[in] file Strumień, z którego wczytać drzewo.
 * @return Wczytane drzewo.
 */
struct trie_node * trie_deserialize(FILE *file);

/**
 * Zwraca dziecko o podanej wartości.
 * 
 * @param[in] node Węzeł, którego dziecko zwrócić.
 * @param[in] value Wartość dziecka.
 * @return Węzeł reprezentujący dziecko lub NULL jeśli takowe nie istnieje.
 */
const struct trie_node * trie_get_child(const struct trie_node *node, wchar_t value);

/**
 * Zwraca dzieci danego węzła.
 * Tablica może stać się niepoprawna po modyfikujących drzewo operacjach.
 * @param[in] node Węzeł, którego dzieci zwrócić.
 * @param[out] children Tablica wskaźników na dzieci.
 * @return Liczba dzieci w tablicy.
 */
int trie_get_children(const struct trie_node *node, const struct trie_node *** children);

/**
 * Zwraca wartość przechowywaną w danym węźle.
 * @param[in] node Węzeł, którego wartość zwrócić.
 * @return Wartość węzła.
 */
wchar_t trie_get_value(const struct trie_node *node);

/**
 * Zwraca wskaźnik na wartość przechowywaną w danym węźle.
 * @param[in] node Węzeł.
 * @return Wskaźnik na Wartość węzła.
 */
const wchar_t * trie_get_value_ptr(const struct trie_node *node);

/**
 * Sprawdza, czy węzeł jest liściem.
 * @param[in] node Węzeł.
 * @return Czy węzeł jest liściem.
 */
bool trie_is_leaf(const struct trie_node *node);

/**
 * Znajduje wyrazy podobne do podanego w drzewie.
 * 
 * @param[in] root Drzewo do przeszukania.
 * @param[in] word Słowo wzorcowe, do którego znaleźć podobne.
 * @param[out] list Lista słów podobnych.
 * @param[in] rules Lista reguł, które można zastosować.
 * @param[in] max_cost Maksymalny możliwy koszt podpowiedzi.
 * @param[in] max_hints_no Maksymalna liczba podpowiedzi.
 */
void trie_hints(struct trie_node *root, const wchar_t *word, struct word_list *list, struct list *rules, int max_cost, int max_hints_no);

#endif /* __TRIE_H__ */