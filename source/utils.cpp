#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <sys\stat.h>

#include "utils.h"


void BufNextString(char** buffer)
{
    while (*(*buffer)++ != '\n') {}
}


int DigitsNumber(int num)
{
    return (int) log10((double) num) + 1;
}


CodeError Fsize(const char* file_name, int* input_buffer_length)
{
    struct stat input_file_stat = {};
    if (stat(file_name, &input_file_stat) != 0)
        return FILLING_FSTAT_ERR;

    *input_buffer_length = input_file_stat.st_size + 1;
    return NO_ERROR;
}
