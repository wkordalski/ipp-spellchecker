#include "dictionary.h"
#include <locale.h>
#include <stdio.h>

int main(void)
{
    setlocale(LC_ALL, "en_US.UTF-8");
    struct dictionary *dict = dictionary_new();
    printf("Podaj słowo: ");
    wchar_t buf[64];
    if (scanf("%63ls", buf) <= 0)
    {
        fprintf(stderr, "Failed to read word\n");
        return 1;
    }
    dictionary_insert(dict, buf);
    const wchar_t *word = dictionary_getword(dict);
    if (!word)
    {
        fprintf(stderr, "Dictionary empty!\n");
        return 1;
    }
    if (wcscmp(buf, word))
        printf("Words match\n");
    else
        printf("Mismatch!!\n");
    dictionary_done(dict);
    return 0;
}
