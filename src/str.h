#include <stdbool.h>
#include <stddef.h>

// Simple pointer-to-pointer copy
void cstr_cpy(const char *src, char *dest, size_t len);

// Get length of a C string
size_t cstr_len(const char *s);

// Compare C strings for equality
bool cstr_eql(const char *a, const char *b);

// Get the index of a character in a string
long cstr_indexof(const char *haystack, char needle);

// Check if a C string haystack starts with a needle
bool cstr_starts_with(const char *haystack, const char *needle);

// String Builder
typedef struct SB {
  char *ptr;
  size_t len;
  size_t cap;
} SB;

// Initialize a String Builder with a pointer and capacity
SB SB_fromPtrCap(char *ptr, size_t cap);
// Initialize a String Builder with a pointer, initialized length, and capacity
SB SB_fromPtrLenCap(char *ptr, size_t len, size_t cap);

// Initialize a String Builder from a stack array (calls sizeof)
#define SB_fromArray(arr) SB_fromPtrCap(arr, sizeof(arr))

// Append a string to the String Builder
void SB_appendLen(SB *b, const char *str, size_t len);
// Append a string to the String Builder
void SB_append(SB *b, const char *str);
