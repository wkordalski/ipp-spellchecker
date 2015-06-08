# dodajemy bibliotekę dictionary, stworzoną na podstawie pliku dictionary.c
# biblioteka będzie dołączana statycznie (czyli przez linkowanie pliku .o)

add_library (dictionary dictionary.c word_list.c trie.c charmap.c rule.c list.c)


if (CMOCKA) 
    add_executable (word_list_test word_list.c word_list_test.c ../testable.c)
    target_link_libraries (word_list_test ${CMOCKA})
    add_test (word_list_unit_test word_list_test)
    
    
    add_executable (list_test list_test.c list.c ../testable.c)
    target_link_libraries (list_test ${CMOCKA})
    set_target_properties(list_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    add_test (list_unit_test list_test)
    
    
    add_executable (charmap_test charmap.c charmap_test.c ../testable.c)
    target_link_libraries (charmap_test ${CMOCKA})
    set_target_properties(charmap_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    add_test (charmap_unit_test charmap_test)
    
    
    add_executable (trie_test trie.c trie_test.c charmap.c word_list.c ../testable.c)
    target_link_libraries (trie_test ${CMOCKA})
    set_target_properties(trie_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    add_test (trie_unit_test trie_test)
    
    
    add_executable (dictionary_test dictionary_test.c dictionary.c word_list.c trie.c charmap.c ../testable.c)
    target_link_libraries (dictionary_test ${CMOCKA})
    set_target_properties(dictionary_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    add_test (dictionary_unit_test dictionary_test)
endif (CMOCKA)
