/** @file
  Implementacja pomocniczych funkcji do testowania.
  
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwersytet Warszawski
  @date 2015-05-31
 */

#include <wchar.h>

unsigned char buff[1024];
int readp = 0;
int writep = 0;
int filelen = 0;

int test_fputc(unsigned char c)
{
    if(writep >= 1024) return -1;
    buff[writep++] = c;
    if(writep > filelen) filelen = writep;
    return 0;
}


int test_fputs(char *s)
{
    while(*s)
    {
        if(test_fputc(*s) < 0) return -1;
        s++;
    }
    return 0;
}

int test_fgetc()
{
    if(readp >= filelen) return -1;
    return buff[readp++];
}

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


wchar_t wbuff[1024];
int wreadp = 0;
int wwritep = 0;
int wfilelen = 0;

int test_fputwc(wchar_t c)
{
    if(wwritep >= 1024) return -1;
    wbuff[wwritep++] = c;
    if(wwritep > wfilelen) wfilelen = wwritep;
    return 0;
}

int test_fgetwc()
{
    if(wreadp >= wfilelen) return -1;
    return wbuff[wreadp++];
}


