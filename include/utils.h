/*!
    \file
    File with utilities
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "settings.h"

void BufNextString(char** buffer);
CodeError fsize(const char* file_name, int* input_buffer_length);

#endif
