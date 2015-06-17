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

int int32_serialize(int value, FILE *f);

int int32_deserialize(int *value, FILE *f);

#endif /* DICTIONARY_SERIALIZATION_H */