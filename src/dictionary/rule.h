/** @file
    Interfejs podpowiedzi.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-02
 */

#ifndef DICTIONARY_RULE_H
#define DICTIONARY_RULE_H

/**
 * Reguła podpowiadania słów z drzewa.
 */
struct hint_rule;

#include "dictionary.h"
#include "list.h"
#include "trie.h"

/**
 * Tworzy regułę.
 * 
 * @param[in] src Wzorzec do dopasowania.
 * @param[in] dst Tekst docelowy do wstawienia za wzorzec.
 * @param[in] cost Koszt reguły.
 * @param[in] flag Flagi reguły.
 * @return Wskaźnik na nową regułę.
 */
struct hint_rule * rule_make(const wchar_t *src, const wchar_t *dst, int cost, enum rule_flag flag);

/**
 * Usuwa regułę.
 * 
 * @param[in,out] rule Reguła do usunięcia.
 */
void rule_done(struct hint_rule *rule);

/**
 * Generuje podpowiedzi do słowa używając danych reguł.
 * 
 * @param[in] rules Tablica wskaźnikóœ na reguły zakończona NULL-em.
 * @param[in] max_cost Maksymalny koszt podpowiedzi.
 * @param[in] max_hints_no Maksymalna liczba podpowiedzi.
 * @param[in] root Korzeń drzewa TRIE.
 * @param[in] word Słowo, dla którego wygenerować podpowiedzi.
 * @return Listę podpowiedzi.
 */
struct list * rule_generate_hints(struct hint_rule **rules, int max_cost, int max_hints_no, struct trie_node *root, const wchar_t *word);

/**
 * Zapisuje regułę do pliku.
 * 
 * @param[in] rule Reguła.
 * @param[in] file Plik.
 * @return <0 jeśli błąd, 0 w p.p.
 */
int rule_serialize(struct hint_rule *rule, FILE *file);

/**
 * Wczytuje regułę z pliku.
 * 
 * @param[in] file Plik.
 * @return Reguła lub NULL jeśli błąd.
 */
struct hint_rule *rule_deserialize(FILE *file);

#endif /* DICTIONARY_RULE_H */