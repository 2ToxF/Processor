#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
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
