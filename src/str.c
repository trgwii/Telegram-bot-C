#include "str.h"

void str_cpy(const char *src, char *dest, unsigned long len) {
  for (unsigned long i = 0; i < len; i++) {
    *dest++ = *src++;
  }
}

unsigned long str_len(const char *s) {
  unsigned long len = 0;
  while (*s++)
    len++;
  return len;
}

bool str_eql(const char *a, const char *b) {
  if (a == b)
    return true;
  while (*a && *b && *a++ == *b++)
    ;
  return !*a && !*b;
}

bool str_starts_with(const char *haystack, const char *needle) {
  while (*haystack && *needle && *haystack++ == *needle++)
    ;
  return !*needle;
}
