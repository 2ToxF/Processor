#include <stdio.h>
#include <stdlib.h>

#include "spu.h"

static const char INPUT_FILE_CODE_NAME = "inpur_code.txt";
static const int NUM_OF_ACTIONS_PER_CHAR = 1;

CodeError CodeAssmble(const char input_file_name[])
{
    CodeError code_err = NO_ERROR;

    char* input_code_buf = NULL;
    int input_code_buf_len = 0;
    if ((code_err = my_fread(&input_code_buf, &input_code_buf_len, input_file_name)) != NO_ERROR)
        return code_err;


}


CodeError MyFread(char** input_buffer, int* input_buffer_length, char* input_file_name)
{
    CodeError code_err = NO_ERROR;

    FILE* input_fptr = fopen(input_file_name, "r");
    if (input_fptr == NULL)
        return FILE_NOT_OPENED_ERR;

    if ((code_err = fsize(input_file_name, input_buffer_length)) != NO_ERROR)
        return code_err;

    *input_buffer = (char*) calloc(*input_buffer_length, sizeof(char));
    long long unsigned int success_read_string_length = fread(*input_buffer, NUM_OF_ACTIONS_PER_CHAR,
                                                              *input_buffer_length - 1, input_fptr);
    if (success_read_string_length != (long long unsigned int) (*input_buffer_length - 1))
        return WRONG_BUFSIZE_ERR;

    fclose(input_fptr); input_fptr = NULL;
    return code_err;
}


CodeError RunProgram()
{
    CodeAssemble(FILE_FOR_ASM_NAME);

    return NO_ERROR;
}
