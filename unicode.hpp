#pragma once

#include <exception>
#include <stdexcept>

int UTF8Encode(int codePoint, char *out);
int UTF8Decode(int* in, int& out);

int UTF16Encode(int codePoint, char16_t* out);
int UTF16Decode(int* in, int& out);

int UTF32Encode(int codePoint, char32_t* out);
int UTF32Decode(int* in, int& out);
