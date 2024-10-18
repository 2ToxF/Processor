/*!
    \file
    File with utilities
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "settings.h"

void BufNextString(char** buffer);
int DigitsNumber(int num);
CodeError Fsize(const char* file_name, int* input_buffer_length);

#endif
