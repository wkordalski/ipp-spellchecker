/*
 * Copyright 2008 Google Inc.
 * Copyright 2015 Tomasz Kociumaka
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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



/* All functions in this object need to be exposed to the test application,
 * so redefine static to nothing. */
#define static

#endif /* UNIT_TESTING */
#endif /* TESTABLE_H*/

