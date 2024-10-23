/*!
    \file
    File with utilities
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "settings.h"

void      BufNextString(char** buffer);
void      BufNextWord  (char** buffer);
void      BufSkipSpaces(char** buffer);
CodeError Fsize        (const char* file_name, int* input_buffer_length);
bool      StrIsNum     (const char* str);

#endif
