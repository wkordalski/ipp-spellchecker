/** @defgroup dict-editor Moduł dict-editor.
    Prosty klient biblioteki dictionary.
    Umożliwia edycję słownika.
  */
/** @file
    Główny plik modułu dict-editor
    @ingroup dict-editor
    @author Jakub Pawlewicz <pan@mimuw.edu.pl>
    @date 2015-05-11
    @copyright Uniwersytet Warszawski
    @todo Poprawić proste parsowanie na porządniejsze.
  */

#include "dictionary.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

enum {
    INSERT,
    DELETE,
    FIND,
    HINTS,
    SAVE,
    LOAD,
    QUIT,
    CLEAR,
    COMMANDS_COUNT };

static const char *commands[] =
{
    "insert",
    "delete",
    "find",
    "hints",
    "save",
    "load",
    "quit",
    "clear"
};

void skip_line()
{
    scanf("%*[^\n]\n");
}

int ignored()
{
    printf("ignored\n");
    skip_line();
    return 1;
}

int make_lowercase(wchar_t *word)
{
    for (wchar_t *w = word; *w; ++w)
        if (!iswalpha(*w))
            return 0;
        else
            *w = towlower(*w);
    return 1;
}

int try_process_command(struct dictionary *dict)
{
    char cmd[16];
    if (scanf("%15s", cmd) <= 0)
    {
        if (ferror(stdin))
        {
            fprintf(stderr, "Failed to read command\n");
            exit(1);
        }
        return 0;
    }
    int c;
    for (c = 0; c < COMMANDS_COUNT; ++c)
        if (!strcmp(cmd, commands[c]))
            break;
    if (c == COMMANDS_COUNT)
    {
        fprintf(stderr, "Invalid command '%s'\n", cmd);
        return ignored();
    }
    else if (c == QUIT)
        return 0;
    else if (c == CLEAR)
    {
        dictionary_done(dict);
        dict = dictionary_new();
        printf("cleared\n");
    }
    else if (c < SAVE)
    {
        wchar_t word[64];
        if (scanf("%63ls", word) <= 0)
        {
            fprintf(stderr, "Failed to read word\n");
            exit(1);
        }
        if (!make_lowercase(word))
        {
            fprintf(stderr, "Invalid word '%ls'\n", word);
            return ignored();
        }
        switch (c)
        {
            case INSERT:
                if (dictionary_insert(dict, word))
                    printf("inserted: %ls\n", word);
                else
                    return ignored();
                break;
            case DELETE:
                if (dictionary_delete(dict, word))
                    printf("deleted: %ls\n", word);
                else
                    return ignored();
                break;
            case FIND:
                if (dictionary_find(dict, word))
                    printf("found: %ls\n", word);
                else
                    printf("not found: %ls\n", word);
                break;
            case HINTS:
            {
                struct word_list list;
                dictionary_hints(dict, word, &list);
                const wchar_t * const *a = word_list_get(&list);
                for (size_t i = 0; i < word_list_size(&list); ++i)
                {
                    if (i)
                        printf(" ");
                    printf("%ls", a[i]);
                }
                printf("\n");
                break;
            }
        }
    }
    else
    {
        char filename[512];
        if (scanf("%511s", filename) <= 0)
        {
            fprintf(stderr, "Failed to read filename\n");
            exit(1);
        }
        switch (c)
        {
            case SAVE:
            {
                FILE *f = fopen(filename, "w");
                if (!f || dictionary_save(dict, f))
                {
                    fprintf(stderr, "Failed to save dictionary\n");
                    exit(1);
                }
                fclose(f);
                printf("dictionary saved in file %s\n", filename);
                break;
            }
            case LOAD:
            {
                FILE *f = fopen(filename, "r");
                struct dictionary *new_dict;
                if (!f || !(new_dict = dictionary_load(f)))
                {
                    fprintf(stderr, "Failed to load dictionary\n");
                    exit(1);
                }
                fclose(f);
                printf("dictionary loaded from file %s\n", filename);
                dictionary_done(dict);
                dict = new_dict;
                break;
            }
        }
    }
    skip_line();
    return 1;
}

/**
  Funkcja main. 
  Służy do testowania słownika przechowującego jedno słowo. 
  */
int main(void)
{
    setlocale(LC_ALL, "pl_PL.UTF-8");
    struct dictionary *dict = dictionary_new();
    do {} while (try_process_command(dict));
    dictionary_done(dict);
    return 0;
}
