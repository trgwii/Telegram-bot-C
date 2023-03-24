#include "str.h"

void cstr_cpy(const char *src, char *dest, unsigned long len) {
  for (unsigned long i = 0; i < len; i++) {
    *dest++ = *src++;
  }
}

unsigned long cstr_len(const char *s) {
  unsigned long len = 0;
  while (*s++)
    len++;
  return len;
}

bool cstr_eql(const char *a, const char *b) {
  if (a == b)
    return true;
  while (*a && *b && *a++ == *b++)
    ;
  return !*a && !*b;
}

bool cstr_starts_with(const char *haystack, const char *needle) {
  while (*haystack && *needle && *haystack++ == *needle++)
    ;
  return !*needle;
}

Str Str_fromC(char *cstr) {
  Str str = {
      .ptr = cstr,
      .len = cstr_len(cstr),
  };
  return str;
}

Str Str_fromPtrLen(char *ptr, size_t len) {
  Str str = {
      .ptr = ptr,
      .len = len,
  };
  return str;
}

SB SB_fromPtrLen(char *ptr, size_t len, void *(*realloc)(void *, size_t)) {
  SB b = {
      .str = {.ptr = ptr, .len = 0},
      .cap = len,
      .realloc = realloc,
  };
  return b;
}

void SB_zend(SB *b) { ((char *)b->str.ptr)[b->str.len] = 0; }

void SB_append(SB *b, Str str) {
  if (b->str.len + str.len + 1 >= b->cap) {
    if (!b->realloc)
      return;
    size_t new_cap = b->cap ? b->cap : 16;
    while (new_cap < b->str.len + str.len + 1)
      new_cap *= 2;
    void *new_ptr = b->realloc(b->str.ptr, new_cap);
    if (!new_ptr)
      return;
    b->str.ptr = new_ptr;
    b->cap = new_cap;
  }
  cstr_cpy(str.ptr, b->str.ptr + b->str.len, str.len);
  b->str.len += str.len;
  SB_zend(b);
}

void SB_appendC(SB *b, char *cstr) { SB_append(b, Str_fromC(cstr)); }

void SB_shrink(SB *b) {
  if (b->realloc && b->cap > b->str.len + 1) {
    void *new_ptr = b->realloc(b->str.ptr, b->str.len + 1);
    if (!new_ptr)
      return;
    b->cap = b->str.len + 1;
    SB_zend(b);
  }
}
