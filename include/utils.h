/*!
    \file
    File with utilities
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "settings.h"

void      BufNextSpace (char** buffer);
void      BufNextString(char** buffer);
void      BufNextWord  (char** buffer);
int       DigitsNumber (int num);
CodeError Fsize        (const char* file_name, int* input_buffer_length);
bool      StrIsNum     (const char* str);

#endif
