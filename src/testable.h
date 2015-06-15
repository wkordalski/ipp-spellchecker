/** @file
  Nagłówek modyfikujący źródła w celu testowania.
  
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Google Inc.
  @copyright Uniwersytet Warszawski
  @date 2015-05-31
 */

#ifndef TESTABLE_H
#define TESTABLE_H
/* If this is being built for a unit test. */
#ifdef UNIT_TESTING

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include <wchar.h>

/* Redirect assert to mock_assert() so assertions can be caught by cmocka. */
#ifdef assert
#undef assert
#endif /* assert */
#define assert(expression) \
    mock_assert((int)(expression), #expression, __FILE__, __LINE__)
void mock_assert(const int result, const char* expression, const char *file,
                 const int line);

#ifdef malloc
#undef malloc
#endif /* malloc */
#define malloc(size) _test_malloc(size, __FILE__, __LINE__)
void * _test_malloc(const size_t size, const char* file, const int line);

#ifdef free
#undef free
#endif /* free */
#define free(ptr) _test_free(ptr, __FILE__, __LINE__)
void _test_free(void* const ptr, const char* file, const int line);


extern unsigned char buff[1024];
extern int readp;
extern int writep;
extern int filelen;

#ifdef fputc
#undef fputc
#endif
#define fputc(char, file) test_fputc(char)
int test_fputc(unsigned char c);

#ifdef fputs
#undef fputs
#endif
#define fputs(str, file) test_fputs(str)
int test_fputs(char *s);

#ifdef fgetc
#undef fgetc
#endif
#define fgetc(file) test_fgetc()
int test_fgetc();

#ifdef fgets
#undef fgets
#endif
#define fgets(str, cnt, file) test_fgets(str, cnt)
char* test_fgets(char *str, int cnt);

extern wchar_t wbuff[1024];
extern int wreadp;
extern int wwritep;
extern int wfilelen;

#ifdef fputwc
#undef fputwc
#endif
#define fputwc(char, file) test_fputwc(char)
int test_fputwc(wchar_t c);

#ifdef fgetwc
#undef fgetwc
#endif
#define fgetwc(file) test_fgetwc()
int test_fgetwc();


/* All functions in this object need to be exposed to the test application,
 * so redefine static to nothing. */
#define static

#endif /* UNIT_TESTING */
#endif /* TESTABLE_H*/

