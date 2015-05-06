/** @file
    Interfejs biblioteki obsługującej słownik.
    @author Jakub Pawlewicz <pan@mimuw.edu.pl>
    @copyright Uniwerstet Warszawski
    @date 2015-05-06
 */

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <wchar.h>


/**
  Struktura przechowująca słownik.
  */
struct dictionary;


/**
  Inicjalizacja słownika.
  @param[in,out] dictionary Słownik.
  */
void dictionary_init(struct dictionary*);


/**
  Destrukcja słownika.
  @param[in,out] dictionary Słownik.
  */
void dictionary_done(struct dictionary*);

/**
  Wstawia podane słowo do słownika.
  @param[in,out] dictionary Słownik.
  @param[in] word Słowo, które należy wstawić do słownik.
  */
void dictionary_insert(struct dictionary*, const wchar_t*);


/**
  Zwraca słowo znajdujące się w słowniku.
  @param[in,out] dictionary Słownik.
  @return Słowo znajdujące się w słowniku
  */

const wchar_t* dictionary_getword(struct dictionary *);

#endif /* __DICTIONARY_H__ */
