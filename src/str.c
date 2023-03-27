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

SB SB_fromPtrLenCap(char *ptr, size_t len, size_t cap) {
  SB b = {
      .ptr = ptr,
      .len = len,
      .cap = cap,
  };
  return b;
}

SB SB_fromPtrCap(char *ptr, size_t cap) {
  SB b = {
      .ptr = ptr,
      .len = 0,
      .cap = cap,
  };
  return b;
}

void SB_zend(SB *b) { b->ptr[b->len] = 0; }

void SB_append(SB *b, const char *str) { SB_appendLen(b, str, cstr_len(str)); }

void SB_appendLen(SB *b, const char *str, size_t len) {
  if (b->len + len + 1 >= b->cap)
    return;
  cstr_cpy(str, b->ptr + b->len, len);
  b->len += len;
  SB_zend(b);
}
