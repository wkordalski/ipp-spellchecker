cmake_minimum_required ( VERSION 2.6 )
project (Dictionary C)

option(DEBUG OFF)

if (DEBUG)
    set(CMAKE_BUILD_TYPE DEBUG)
else (DEBUG)
    set(CMAKE_BUILD_TYPE RELEASE)
endif (DEBUG)

set(CMAKE_C_FLAGS_DEBUG "-std=c99 -Wall -pedantic -g")
set(CMAKE_C_FLAGS_RELEASE "-std=c99 -O3")

add_executable (dictionary dictionary.c)
target_link_libraries (dictionary parse trie)

add_executable (dictionary_memo dictionary.c)
target_link_libraries (dictionary_memo parse trie_memo)

add_executable (dictionary_brut dictionary.c)
target_link_libraries (dictionary_brut parse trie_brut)

add_library (parse STATIC parse.c)
add_library (trie STATIC trie.c)
add_library (trie_memo STATIC trie_much_memo.c)
add_library (trie_brut STATIC trie_brut.c)




# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc ALL
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            )
endif(DOXYGEN_FOUND)
