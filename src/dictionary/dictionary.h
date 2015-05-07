/** @defgroup dictionary Moduł dictionary
    Biblioteka obsługująca słownik.
  */
/** @file 
    Interfejs biblioteki obsługującej słownik.
   
    @ingroup dictionary
    @author Jakub Pawlewicz <pan@mimuw.edu.pl>
    @copyright Uniwerstet Warszawski
    @date 2015-05-06
    @todo Rozbudować interfejs o obsługę plików.
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
  @param[in] word Słowo, które należy wstawić do słownik.
  */
void dictionary_insert(struct dictionary *dict, const wchar_t* word);


/**
  Zwraca słowo znajdujące się w słowniku.
  @param[in,out] dict Słownik.
  @return Słowo znajdujące się w słowniku.
  */

const wchar_t* dictionary_getword(struct dictionary *dict);

#endif /* __DICTIONARY_H__ */
