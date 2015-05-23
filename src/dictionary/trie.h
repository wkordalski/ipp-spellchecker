
#include <stdio.h>
#include <stdlib.h>

struct trie_node;


struct trie_node * trie_init();

void trie_done(struct trie_node *root);

void trie_clear(struct trie_node *root);

int trie_insert(struct trie_node *root, const wchar_t *word);

int trie_find(struct trie_node *root, const wchar_t *word);

int trie_delete(struct trie_node *root, const wchar_t *word);

void trie_serialize(struct trie_node *root, FILE *file);

struct trie_node * trie_deserialize(FILE *file);