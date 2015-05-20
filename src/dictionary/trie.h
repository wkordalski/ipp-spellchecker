struct trie_node
{
    wchar_t val;                ///< Wartość węzła
    int cnt : 31;               ///< Ilość dzieci
    int leaf : 1;               ///< Czy tutaj kończy się słowo
    struct trie_node **chd;     ///< Lista dzieci
};


struct trie_node * trie_init();

void trie_done(struct trie_node *root);

void trie_clear(struct trie_node *root);

int trie_insert(struct trie_node *node, const wchar_t *word);