#include <stdbool.h>
#include <stddef.h>

void cstr_cpy(const char *src, char *dest, unsigned long len);

unsigned long cstr_len(const char *s);

bool cstr_eql(const char *a, const char *b);

bool cstr_starts_with(const char *haystack, const char *needle);

typedef struct Str {
  char *ptr;
  size_t len;
} Str;

typedef struct SB {
  Str str;
  size_t cap;
  void *(*realloc)(void *, size_t);
} SB;

Str Str_fromC(char *cstr);
Str Str_fromPtrLen(char *ptr, size_t len);

SB SB_fromPtrLen(char *ptr, size_t len, void *(*realloc)(void *, size_t));

#define SB_fromArray(arr) SB_fromPtrLen(arr, sizeof(arr), NULL)
#define SB_dynamic(realloc) SB_fromPtrLen(NULL, 0, realloc)

void SB_append(SB *b, Str str);
void SB_zend(SB *b);
void SB_appendC(SB *b, char *cstr);
void SB_shrink(SB *b);
