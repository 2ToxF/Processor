/*!
    \file
    File with utilities
*/

#ifndef UTILS_H
#define UTILS_H

#include "settings.h"

void SeparateBigString();

CodeError fsize(const char* file_name, int* input_buffer_length)

/*!
    Get random number
    \return Random number as an unsigned long
*/
uint64_t MyGetRandom64();

/*!
    Hashes info using DJB2 algorithm
    \param[in]  ptr              Pointer to the first byte
    \param[in]  number_of_bytes  Number of bytes that should be hashed
    \return Hash <number_of_butes> elements with the first one on <ptr>
*/
unsigned long MyHash(void* ptr, int number_of_bytes);

#endif
