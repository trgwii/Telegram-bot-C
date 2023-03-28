#include "str.h"

void cstr_cpy(const char *src, char *dest, size_t len) {
  for (size_t i = 0; i < len; i++) {
    *dest++ = *src++;
  }
}

size_t cstr_len(const char *s) {
  size_t len = 0;
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
  while (*haystack && *needle && *haystack == *needle) {
    haystack++;
    needle++;
  }
  return !*needle;
}

long cstr_indexof(const char *haystack, char needle) {
  long i = 0;
  while (haystack[i]) {
    if (haystack[i] == needle)
      return i;
    i++;
  }
  return -1;
}

SB SB_fromPtrLenCap(char *ptr, size_t len, size_t cap) {
  return (SB){
      .ptr = ptr,
      .len = len,
      .cap = cap,
  };
}

SB SB_fromPtrCap(char *ptr, size_t cap) {
  return (SB){
      .ptr = ptr,
      .len = 0,
      .cap = cap,
  };
}

void SB_append(SB *b, const char *str) { SB_appendLen(b, str, cstr_len(str)); }

void SB_appendLen(SB *b, const char *str, size_t len) {
  if (b->len + len + 1 >= b->cap)
    return;
  cstr_cpy(str, b->ptr + b->len, len);
  b->len += len;
  b->ptr[b->len] = 0;
}
