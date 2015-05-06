#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <wchar.h>

struct dictionary;

int dictionary_insert(struct dictionary*, const wchar_t*);

#endif /* __DICTIONARY_H__ */
