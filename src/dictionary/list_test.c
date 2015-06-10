/** @file
  Test implementacji listy.
  
  @ingroup dictionary
  @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
          
  @copyright Uniwerstet Warszawski
  @date 2015-05-31
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "list.h"

static void empty(void **s){}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(empty)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
