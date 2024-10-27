#include <stdio.h>
#include <stdlib.h>

#include "input_output.h"
#include "utils.h"


CodeError MyFread(char** input_buffer, int* input_buffer_length, const char* input_file_name)
{
    CodeError code_err = NO_ERROR;

    FILE* input_fptr = fopen(input_file_name, "rb");
    if (input_fptr == NULL)
        return FILE_NOT_OPENED_ERR;

    if ((code_err = Fsize(input_file_name, input_buffer_length)) != NO_ERROR)
        return code_err;

    *input_buffer = (char*) calloc(*input_buffer_length, sizeof(char));
    long long unsigned int success_read_string_length = fread(*input_buffer, 1, *input_buffer_length - 1, input_fptr);
    if (success_read_string_length != (long long unsigned int) (*input_buffer_length - 1))
        return WRONG_BUFSIZE_ERR;

    fclose(input_fptr); input_fptr = NULL;
    return code_err;
}


void PrintCodeError(CodeError code_err)
{
    #define ERR_DESCR_(errcode)                       \
        case errcode:                                 \
            printf(RED "ERROR: " #errcode WHT "\n");  \
            break

    putchar('\n');

    switch (code_err)
    {
        case NO_ERROR:
            printf(GRN "Code was done without errors" WHT "\n");
            break;

        ERR_DESCR_(FILE_NOT_OPENED_ERR);
        ERR_DESCR_(FILLING_FSTAT_ERR);
        ERR_DESCR_(WRONG_BUFSIZE_ERR);

        ERR_DESCR_(OUT_OF_MEM_ERR);
        ERR_DESCR_(TOO_MUCH_MARKS_ERR);
        ERR_DESCR_(UNKNOWN_ARG_ASM_ERR);
        ERR_DESCR_(UNKNOWN_CMD_ASM_ERR);
        ERR_DESCR_(UNKNOWN_MARK_NAME_ERR);
        ERR_DESCR_(UNKNOWN_REG_NAME_ERR);

        ERR_DESCR_(STACK_ERR);
        ERR_DESCR_(UNKNOWN_RUNTIME_CMD_ERR);

        default:
            printf(RED "UNKNOWN ERROR" WHT);
            break;
    }

    #undef ERR_DESCR_
}
