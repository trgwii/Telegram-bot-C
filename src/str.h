#include <stdbool.h>

void str_cpy(const char *src, char *dest, unsigned long len);

unsigned long str_len(const char *s);

bool str_eql(const char *a, const char *b);

bool str_starts_with(const char *haystack, const char *needle);
