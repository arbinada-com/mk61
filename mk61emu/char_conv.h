#ifndef CHAR_CONV_H_INCLUDED
#define CHAR_CONV_H_INCLUDED

#include "mk72.h"

#define UTF8_ERROR  ((int)(-2))

EXTERN_C char convert_char_utf8_to_windows1251(const int code_utf8);
EXTERN_C int convert_utf8_to_windows1251(const char *utf8, char *windows1251, const size_t max_len);
EXTERN_C int convert_windows1251_to_utf8(const char *windows1251, char *utf8, const size_t max_len);
EXTERN_C int transliterate_from_windows1251(const char *src, char *dest, const size_t max_len);

#endif // CHAR_CONV_H_INCLUDED
