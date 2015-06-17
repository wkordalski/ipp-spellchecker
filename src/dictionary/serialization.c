/** @file
    Implementacja funkcji serializujących.

    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
            
    @copyright Uniwerstet Warszawski
    @date 2015-06-17
 */

#include "serialization.h"

#include <stddef.h>
#include <stdio.h>

int int32_serialize(int value, FILE *f)
{
    if(fputwc(((value>>28)&0xF)+L'a', f)<0) return -1;
    if(fputwc(((value>>24)&0xF)+L'a', f)<0) return -1;
    if(fputwc(((value>>20)&0xF)+L'a', f)<0) return -1;
    if(fputwc(((value>>16)&0xF)+L'a', f)<0) return -1;
    if(fputwc(((value>>12)&0xF)+L'a', f)<0) return -1;
    if(fputwc(((value>>8)&0xF)+L'a', f)<0) return -1;
    if(fputwc(((value>>4)&0xF)+L'a', f)<0) return -1;
    if(fputwc(((value)&0xF)+L'a', f)<0) return -1;
    return 0;
}

int int32_deserialize(int *value, FILE *f)
{
    int v = 0;
    for(int i = 0; i < 8; i++)
    {
        int c = fgetwc(f);
        if(c < 0) return -1;
        v = (v<<4)|((c-L'a')&0xF);
    }
    *value = v;
    return 0;
}
