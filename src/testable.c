/** @file
  Implementacja pomocniczych funkcji do testowania.
  
  @ingroup testing
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwersytet Warszawski
  @date 2015-05-31
 */

unsigned char buff[1024];   ///< Bufor zawierający dane zmockowanego pliku.
int readp = 0;              ///< Pozycja czytania w zmockowanym pliku.
int writep = 0;             ///< Pozycja pisania w zmockowanym pliku.
int filelen = 0;            ///< Rozmiar pliku (przy czytaniu)

/**
 * Mock function for fputc.
 * 
 * @param[in] c Character to write into buffer.
 * 
 * @return -1 if an error occured, 0 otherwise.
 */
int test_fputc(unsigned char c)
{
    if(writep >= 1024) return -1;
    buff[writep++] = c;
    if(writep > filelen) filelen = writep;
    return 0;
}

/**
 * Mock function for fputs.
 * 
 * @param[in] s String to write into buffer.
 * 
 * @return -1 if an error occured, 0 otherwise.
 */
int test_fputs(char *s)
{
    while(*s)
    {
        if(test_fputc(*s) < 0) return -1;
        s++;
    }
    return 0;
}

/**
 * Mock function for fgetc.
 * 
 * @returns -1 if an error occured, read character otherwise.
 */
int test_fgetc()
{
    if(readp >= filelen) return -1;
    return buff[readp++];
}

/**
 * Mock function for fgets.
 * 
 * @param[in,out] str Place where to write read characters.
 * @param[in] cnt Number of characters to read,
 * @returns NULL if an error occured, str otherwise.
 */
char* test_fgets(char *str, int cnt)
{
    // Null character counted
    cnt--;
    char *s = str;
    while(cnt--)
    {
        int r = test_fgetc();
        if(r < 0) return 0;
        *s = (unsigned char)r;
        s++;
    }
    return str;
}