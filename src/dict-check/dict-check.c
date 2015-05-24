/** @defgroup dict-check Moduł dict-check
    Program sprawdzający podany tekst pod kątem poprawności.
  */
/** @file
    Główny plik modułu dict-check
    @ingroup dict-check
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
    @date 2015-05-23
    @copyright Uniwersytet Warszawski
  */

#include "dictionary.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>


/**
 * Stany parsowania lini komend.
 */
enum ProgramOptionsParsingState
{
    PositionalParameters
};


/**
  Funkcja main.
  @param[in] argc Liczba parametrów linii komend
  @param[in] argv Lista argumentów linii komend
  @return 0 jeśli program zakończył się powodzeniem, 1 jeśli nastąpił błąd
 */
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "pl_PL.UTF-8");
    
    // Opcje linii komend
    int verbose = 0;
    char *dictfile = NULL;
    enum ProgramOptionsParsingState pars = PositionalParameters;
    for(int i = 1; i < argc; i++)
    {
        switch(pars)
        {
            case PositionalParameters:
            {
                if(strcmp("-v", argv[i]) == 0) verbose = 1;
                else if(dictfile == NULL) dictfile = argv[i];
                else
                {
                    printf("Unknown command line option.\n");
                    printf(" %s [-v] <dictionary file>\n", argv[0]);
                    return 1;
                }
                break;
            }
        }
    }
    
    // Otwieranie konkretnego słownika
    if(dictfile == NULL)
    {
        printf("Dictionary file not specified.\n");
        printf(" %s [-v] <dictionary file>\n", argv[0]);
        return 1;
    }
    FILE *fdict = fopen(dictfile, "rb");
    if(fdict == NULL)
    {
        printf("Could not open dictionary file: %s\n", dictfile);
        return 1;
    }
    struct dictionary *dict = dictionary_load(fdict);
    if(dict == NULL)
    {
        printf("Could not parse dictionary file.\n");
        return 1;
    }
    fclose(fdict);
    
    
    // Przetwarzanie tekstu do sprawdzenia.
    FILE *data = stdin;
    wint_t ch;
    int cap = 1024;
    int len = 0;
    wchar_t *word = malloc(sizeof(wchar_t)*cap);
    wchar_t *lowr = malloc(sizeof(wchar_t)*cap);
    int row = 1;
    int col = 1;
    int ccl = 1;
    struct word_list words;
    while((ch = fgetwc(data)) != EOF)
    {
        if(iswalpha(ch))
        {
            // Dodaj kolejną literę słowa do bufora
            if(len == 0) ccl = col;
            word[len] = ch;
            lowr[len] = towlower(ch);
            col++;
            len++;
            if(len >= cap)
            {
                cap *= 2;
                wchar_t *nw = malloc(sizeof(wchar_t)*cap);
                wchar_t *nl = malloc(sizeof(wchar_t)*cap);
                memcpy(nw, word, sizeof(wchar_t)*len);
                memcpy(nl, lowr, sizeof(wchar_t)*len);
                free(word); free(lowr);
                word = nw; lowr = nl;
            }
        }
        else
        {
            if(len > 0)
            {
                // Sprawdź słowo z bufora
                word[len] = 0;
                lowr[len] = 0;
                if(!dictionary_find(dict, lowr))
                {
                    printf("#%ls", word);
                    if(verbose)
                    {
                        fprintf(stderr, "%d,%d %ls:", row, ccl, word);
                        //word_list_init(&words); -- initialized in dictionary_hints
                        dictionary_hints(dict, lowr, &words);
                        const wchar_t * const * arr = word_list_get(&words);
                        for(int i = 0; i < word_list_size(&words); i++)
                        {
                            fprintf(stderr, " %ls", arr[i]);
                        }
                        word_list_done(&words);
                        fprintf(stderr, "\n");
                    }
                }
                else
                {
                    printf("%ls", word);
                }
                len = 0;
            }
            // Ogarnianie wiersza i kolumny
            if(ch == L'\n')
            {
                row++;
                col = 1;
            }
            else
            {
                col++;
            }
            // Wypisywanie znaków na wyjście
            printf("%lc", ch);
        }
    }
    // Usuwanie buforów i słownika
    free(word);
    free(lowr);
    dictionary_done(dict);
    return 0;
}
