/*!
    \file
    File with utilities
*/

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#include "settings.h"

void      BufNextString(char** buffer);
void      BufNextWord  (char** buffer);
void      BufSkipSpaces(char** buffer);
CodeError Fsize        (const char* file_name, int* input_buffer_length);
void*     MyRecalloc   (void* mem_ptr, size_t new_size, size_t endof_used_mem);
bool      StrIsNum     (const char* str);

#endif
