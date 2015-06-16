# dodajemy bibliotekę dictionary, stworzoną na podstawie pliku dictionary.c
# biblioteka będzie dołączana statycznie (czyli przez linkowanie pliku .o)

add_library (dictionary dictionary.c word_list.c trie.c rule.c list.c str.c)


if (CMOCKA) 
    add_executable (word_list_test word_list.c word_list_test.c ../testable.c)
    target_link_libraries (word_list_test ${CMOCKA})
    add_test (word_list_unit_test word_list_test)
    
    
    add_executable (list_test list_test.c list.c ../testable.c)
    target_link_libraries (list_test ${CMOCKA})
    set_target_properties(list_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    add_test (list_unit_test list_test)
    
        
    add_executable (rule_test rule_test.c trie.c word_list.c list.c rule.c str.c ../testable.c)
    target_link_libraries (rule_test ${CMOCKA})
    set_target_properties(rule_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    add_test (rule_unit_test rule_test)
    
    
    add_executable (trie_test trie.c trie_test.c word_list.c list.c rule.c str.c ../testable.c)
    target_link_libraries (trie_test ${CMOCKA})
    set_target_properties(trie_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    add_test (trie_unit_test trie_test)
    
    
    add_executable (dictionary_test dictionary_test.c dictionary.c word_list.c trie.c list.c rule.c str.c ../testable.c)
    target_link_libraries (dictionary_test ${CMOCKA})
    set_target_properties(dictionary_test PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
    add_test (dictionary_unit_test dictionary_test)
endif (CMOCKA)
