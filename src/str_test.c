#include "str.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define SB_Debug(b)                                                            \
  printf("ptr %p, len %lu, cap %lu, '%s'\n", (void *)b.ptr, b.len, b.cap, b.ptr)

int main(void) {
  char arr[1024];
  SB b1 = SB_fromArray(arr);

  SB_append(&b1, "Hello");
  SB_append(&b1, ", ");
  SB_append(&b1, "World");
  SB_append(&b1, "!");

  assert(b1.ptr[b1.len] == 0);
  assert(b1.len == 13);
  assert(b1.cap == 1024);
  assert(cstr_eql(b1.ptr, "Hello, World!"));

  SB_Debug(b1);

  return 0;
}
