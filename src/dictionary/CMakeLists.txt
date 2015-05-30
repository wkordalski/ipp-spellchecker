# dodajemy bibliotekę dictionary, stworzoną na podstawie pliku dictionary.c
# biblioteka będzie dołączana statycznie (czyli przez linkowanie pliku .o)

add_library (dictionary dictionary.c word_list.c trie.c charmap.c)


if (CMOCKA)
    # dodajemy plik wykonywalny z testem    
    add_executable (word_list_test word_list.c word_list_test.c)

    # i linkujemy go z biblioteką do testowania
    target_link_libraries (word_list_test ${CMOCKA})

    # wreszcie deklarujemy, że to test
    add_test (word_list_unit_test word_list_test)
    
    
    # dodajemy plik wykonywalny z testem    
    add_executable (charmap_test charmap.c charmap_test.c)

    # i linkujemy go z biblioteką do testowania
    target_link_libraries (charmap_test ${CMOCKA})

    # włącz testable.h
    set_target_properties(charmap_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    
    # wreszcie deklarujemy, że to test
    add_test (charmap_unit_test charmap_test)
    
    
    # dodajemy plik wykonywalny z testem    
    add_executable (trie_test trie.c trie_test.c charmap.c word_list.c)

    # i linkujemy go z biblioteką do testowania
    target_link_libraries (trie_test ${CMOCKA})

    # włącz testable.h
    set_target_properties(trie_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    
    # wreszcie deklarujemy, że to test
    add_test (trie_unit_test trie_test)
endif (CMOCKA)
