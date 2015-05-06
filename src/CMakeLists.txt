cmake_minimum_required ( VERSION 2.6 )
project (Spellcheck C)

option(DEBUG OFF)

if (DEBUG)
    set(CMAKE_BUILD_TYPE DEBUG)
else (DEBUG)
    set(CMAKE_BUILD_TYPE RELEASE)
endif (DEBUG)

set(CMAKE_C_FLAGS_DEBUG "-std=gnu99 -Wall -pedantic -g")
set(CMAKE_C_FLAGS_RELEASE "-std=gnu99 -O3")

include_directories (dictionary)
add_subdirectory (dictionary)
add_subdirectory (dictionary_test)


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc ALL
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            )
endif(DOXYGEN_FOUND)
