#include <stdbool.h>
#include <stddef.h>

void cstr_cpy(const char *src, char *dest, unsigned long len);

unsigned long cstr_len(const char *s);

bool cstr_eql(const char *a, const char *b);

bool cstr_starts_with(const char *haystack, const char *needle);

typedef struct SB {
  char *ptr;
  size_t len;
  size_t cap;
} SB;

SB SB_fromPtrCap(char *ptr, size_t cap);
SB SB_fromPtrLenCap(char *ptr, size_t len, size_t cap);

#define SB_fromArray(arr) SB_fromPtrCap(arr, sizeof(arr))
#define SB_dynamic(realloc) SB_fromPtrCap(NULL, 0)

void SB_appendLen(SB *b, const char *str, size_t len);
void SB_append(SB *b, const char *str);
void SB_zend(SB *b);
