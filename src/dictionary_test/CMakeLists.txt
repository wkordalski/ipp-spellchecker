# deklarujemy plik wykonywalny tworzony na podstawie odpowiedniego pliku źródłowego
add_executable (dictionary_test dictionary_test.c)

# przy kompilacji progamu należy dołączyć bibliotekę
target_link_libraries (dictionary_test dictionary)
