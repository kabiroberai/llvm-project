// RUN: %clang_cc1 -fsyntax-only -fbounds-safety -verify %s -verify-ignore-unexpected=note
// RUN: %clang_cc1 -fsyntax-only -fbounds-safety -x objective-c -fbounds-attributes-objc-experimental -verify %s -verify-ignore-unexpected=note

#include <ptrcheck.h>
#include "system-header-func-decl.h"

// expected-error@+1{{conflicting types for 'foo'}}
int *__single foo(int *__indexable *__single);
