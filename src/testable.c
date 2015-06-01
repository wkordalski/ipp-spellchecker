/** @file
  Implementacja pomocniczych funkcji do testowania.
  
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwersytet Warszawski
  @date 2015-05-31
 */


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