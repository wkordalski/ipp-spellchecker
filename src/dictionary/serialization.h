/** @file
    Interfejs funkcji serializujących.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-17
 */

#ifndef DICTIONARY_SERIALIZATION_H
#define DICTIONARY_SERIALIZATION_H

#include <stdio.h>

/**
 * Zapisuje liczbę 32-bitową do pliku.
 * 
 * @param[in] value Liczba.
 * @param[in] f Plik.
 * @return <0 jeśli błąd, 0 w p.p.
 */
int int32_serialize(int value, FILE *f);

/**
 * Wczytuje liczbę 32-bitową z pliku.
 * 
 * @param[out] value Miejsce gdzie zapisać liczbę.
 * @param[in] f Plik.
 * @return <0 jeśli błąd, 0 w p.p.
 */
int int32_deserialize(int *value, FILE *f);

#endif /* DICTIONARY_SERIALIZATION_H */