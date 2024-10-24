#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h>

#include "utils.h"


void BufNextString(char** buffer)
{
    while (**buffer != '\n')
        ++(*buffer);
    ++(*buffer);

    BufSkipSpaces(buffer);
}


void BufNextWord(char** buffer)
{
    while (**buffer != ' ' && **buffer != '\n')
        ++(*buffer);
    ++(*buffer);

    BufSkipSpaces(buffer);
}


void BufSkipSpaces(char** buffer)
{
    while (**buffer == ' ' || **buffer == '\n' || **buffer == '\r')
        ++(*buffer);
}


CodeError Fsize(const char* file_name, int* input_buffer_length)
{
    struct stat input_file_stat = {};
    if (stat(file_name, &input_file_stat) != 0)
        return FILLING_FSTAT_ERR;

    *input_buffer_length = input_file_stat.st_size + 1;
    return NO_ERROR;
}


void* MyRecalloc(void* mem_ptr, size_t new_size, size_t endof_used_mem)
{
    mem_ptr = realloc(mem_ptr, new_size);
    memset((char*) mem_ptr + endof_used_mem, 0, new_size - endof_used_mem);
    return mem_ptr;
}


bool StrIsNum(const char* str)
{
    int dot_count = 0;

    for (int i = 0; str[i] != '\0'; ++i)
    {
        if (str[i] == '.' && dot_count <= 1)
            ++dot_count;

        else if (!isdigit(str[i]))
            return false;
    }

    return true;
}
