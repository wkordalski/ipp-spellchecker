/** @file
    Interfejs biblioteki obsługującej słownik
    @author Jakub Pawlewicz <pan@mimuw.edu.pl>
    @copyright Uniwerstet Warszawski
    @date 2015-05-06
 */

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <wchar.h>


/**
  @brief Struktura przechowująca słownik.
  */
struct dictionary;

/**
  @brief Wstawia podane słowo do słownika.
  @param[in,out] dictionary Słownik
  @param[in] word Słowo, które należy wstawić do słownik.
  */
int dictionary_insert(struct dictionary*, const wchar_t* word);

#endif /* __DICTIONARY_H__ */
