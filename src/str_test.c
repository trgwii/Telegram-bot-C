#include "str.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define SB_Debug(b)                                                            \
  printf("ptr %p, len %lu, cap %lu, '%s'\n", (void *)b.str.ptr, b.str.len,     \
         b.cap, b.str.ptr)

#define Str_Debug(str)                                                         \
  printf("ptr %p, len %lu, '%s'\n", (void *)str.ptr, str.len, str.ptr)

int main(void) {
  char arr[1024];
  SB b1 = SB_fromArray(arr);

  SB_appendC(&b1, "Hello");
  SB_appendC(&b1, ", ");
  SB_appendC(&b1, "World");
  SB_appendC(&b1, "!");

  assert(b1.str.ptr[b1.str.len] == 0);
  assert(b1.str.len == 13);
  assert(b1.cap == 1024);
  assert(cstr_eql(b1.str.ptr, "Hello, World!"));

  SB b2 = SB_dynamic(realloc);
  SB_appendC(&b2, "Hello");
  SB_appendC(&b2, ", ");
  SB_appendC(&b2, "World");
  SB_appendC(&b2, "!");
  SB_appendC(&b2, " - How are you doing today?");

  assert(b2.str.ptr[b2.str.len] == 0);
  assert(b2.str.len == 40);
  assert(b2.cap == 64);
  assert(cstr_eql(b2.str.ptr, "Hello, World! - How are you doing today?"));

  SB_Debug(b1);
  Str_Debug(b2.str);

  free(b2.str.ptr);

  return 0;
}
