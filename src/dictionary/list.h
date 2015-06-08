/** @file
    Interfejs dynamicznej listy.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-02
 */

#ifndef DICTIONARY_LIST_H
#define DICTIONARY_LIST_H

#include <stddef.h>

/**
  Struktura przechowująca listę.
  */
struct list;

/**
  Inicjuje listę.
  @return Lista lub NULL jeśli alokacja się nie powiodła.
  */
struct list * list_init();

/**
  Destrukcja listy słów.
  @param[in,out] l Lista.
  */
void list_done(struct list *l);

/**
  Dodaje element na koniec listy.
  @param[in,out] l Lista.
  @param[in] e Element.
  @return Indeks wstawionego elementu jeśli się udało, -1 w p.p.
  */
int list_add(struct list *l, void *e);

/**
  Zwraca liczbę elementów w liście.
  @param[in] l Lista.
  @return Liczba elementów w liście.
  */
size_t list_size(const struct list *l);

/**
  Zwraca pojemność listy.
  @param[in] l Lista.
  @return Pojemność listy.
  */
size_t list_capacity(const struct list *l);

/**
 * Rezerwuje ilość miejsca na liście.
 * @param[in] l Lista.
 * @param[in] s Docelowy rozmiar.
 * @return Wynikowa pojemność listy.
 */
size_t list_reserve(struct list *l, size_t s);

/**
  Zwraca tablicę elementów.
  @param[in] l Lista.
  @return Tablica z elementami.
  */
void ** list_get(struct list *l);

/**
 * Sortuje liste elementów.
 * @param[in,out] l Lista.
 * @param[in] f Funkcja sortująca.
 */
void list_sort(struct list *l, int (*f)(void*,void*));

/**
 * Sortuje listę i usuwa duplikaty.
 * @param[in,out] l Lista.
 * @param[in] Funkcja sortująca.
 */
void list_sort_and_unify(struct list *l, int (*f)(void*,void*));

#endif /* DICTIONARY_LIST_H */