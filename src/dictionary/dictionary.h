/** @defgroup dictionary Moduł dictionary
    Biblioteka obsługująca słownik.
  */
/** @file 
    Interfejs biblioteki obsługującej słownik.
   
    @ingroup dictionary
    @author Jakub Pawlewicz <pan@mimuw.edu.pl>
    @copyright Uniwersytet Warszawski
    @date 2015-05-10
 */

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include "word_list.h"
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>

/**
  Struktura przechowująca słownik.
  */
struct dictionary;


/**
  Inicjalizacja słownika.
  Słownik ten należy zniszczyć za pomocą dictionary_done().
  @return Nowy słownik
  */
struct dictionary * dictionary_new(void);


/**
  Destrukcja słownika.
  @param[in,out] dict Słownik.
  */
void dictionary_done(struct dictionary *dict);


/**
  Wstawia podane słowo do słownika.
  @param[in,out] dict Słownik.
  @param[in] word Słowo, które należy wstawić do słownika.
  @return 0 jeśli słowo było już w słowniku, 1 jeśli udało się wstawić.
  */
int dictionary_insert(struct dictionary *dict, const wchar_t* word);


/**
  Usuwa podane słowo ze słownika, jeśli istnieje.
  @param[in,out] dict Słownik.
  @param[in] word Słowo, które należy usunąć ze słownika.
  @return 1 jeśli udało się usunąć, zero jeśli nie.
  */
int dictionary_delete(struct dictionary *dict, const wchar_t* word);


/**
  Sprawdza, czy dane słowo znajduje się w słowniku.
  @param[in] dict Słownik.
  @param[in] word Szukane słowo.
  @return Wartość logiczna czy `word` jest w słowniku.
  */
bool dictionary_find(const struct dictionary *dict, const wchar_t* word);


/**
  Zapisuje słownik.
  @param[in] dict Słownik.
  @param[in,out] stream Strumień, gdzie ma być zapisany słownik.
  @return <0 jeśli operacja się nie powiedzie, 0 w p.p.
  */
int dictionary_save(const struct dictionary *dict, FILE* stream);


/**
  Inicjuje i wczytuje słownik.
  Słownik ten należy zniszczyć za pomocą dictionary_done().
  @param[in,out] stream Strumień, skąd ma być wczytany słownik.
  @return Nowy słownik lub NULL, jeśli operacja się nie powiedzie.
  */
struct dictionary * dictionary_load(FILE* stream);


/**
  Sprawdza, czy dane słowo znajduje się w słowniku.
  @param[in] dict Słownik.
  @param[in] word Szukane słowo.
  @param[in,out] list Lista, w której zostaną umieszczone podpowiedzi.
  */
void dictionary_hints(const struct dictionary *dict, const wchar_t* word,
                      struct word_list *list);

#endif /* __DICTIONARY_H__ */
