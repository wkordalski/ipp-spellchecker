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
#include <stdio.h>

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
  @return Nowy rozmiar listy jeśli się udało, 0 w p.p.
  */
size_t list_add(struct list *l, void *e);

/**
 * Dodaje listę elementów na koniec listy.
 * @param[in,out] l Lista, do której dodać.
 * @param[in] m Lista elementów do dadania.
 * @return Nowy rozmiar listy jeśli się udało, 0 w p.p.
 */
size_t list_add_list(struct list *l, struct list *m);

/**
  Dodaje element na koniec listy.
  @param[in,out] l Lista.
  @param[in] e Element.
  @return Nowy rozmiar listy jeśli się udało, 0 w p.p.
  */
inline size_t list_push(struct list *l, void *e)
{
    return list_add(l, e);
}

/**
 * Usuwa ostatni element z listy.
 * @param[in,out] l Lista.
 * @return Nowy rozmiar listy.
 */
size_t list_pop(struct list *l);

/**
 * Pobiera element z wierzchołka listy.
 * @param[in] l Lista.
 * @return Ostatni element listy.
 */
void * list_top(struct list *l);

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
 * Zmienia rozmiar listy.
 * 
 * @param[in] l Lista.
 * @param[in] s Nowy rozmiar.
 * @param[in] e Domyślny element.
 * 
 * @return Nowy rozmiar listy.
 */
size_t list_resize(struct list *l, size_t s, void *e);

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
void list_sort(struct list* l, int (*f)(const void*, const void*));

/**
 * Sortuje listę i usuwa duplikaty.
 * Wymagane jest aby: f(a,b)=0 => g(a,b)=0, g(a,b)!=0 => f(a,b)=g(a,b)
 * oraz (g(a,c)=0 i f(a,b)<=0 i f(b,c)<=0) => g(b,c)=0.
 * @param[in,out] l Lista.
 * @param[in] f Funkcja sortująca.
 * @param[in] g Funkcja wykazująca równość elementów.
 * @param[in] dups Jeśli różne od NULL, to zostaną tam wstawione duplikaty.
 */
void list_sort_and_unify(struct list* l, int (*f)(const void*, const void*), int (*g)(const void*, const void*), struct list* dups);

/**
 * Usuwa wszystkie elementy z listy.
 * 
 * @param[in,out] l Lista.
 */
void list_clear(struct list *l);

/**
 * Iteruje po wszystkich elementach listy.
 * 
 * @param[in] l Lista.
 * @param[in] a Kontekst.
 * @param[in] f Funkcja do wykonania.
 */

void list_iter(struct list *l, void *a, void (*f)(void *, void *));

int list_serialize(struct list *l, FILE *file, int (*f)(void *, FILE *));

struct list * list_deserialize(FILE *file, void * (*f)(FILE *));

void list_terminate(struct list *l);

#endif /* DICTIONARY_LIST_H */