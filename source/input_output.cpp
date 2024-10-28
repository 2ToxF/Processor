#include <stdio.h>
#include <stdlib.h>

#include "cmd_processing.h"
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
        case CMD_HELP_NO_ERR:
            printf(YEL "Hint was printed, program wasn't started" WHT "\n");
            break;

        case NO_ERROR:
            printf(GRN "Code was done without errors" WHT "\n");
            break;

        ERR_DESCR_(UNKNOWN_OPTION_ERR);

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


void PrintHelp()
{
    printf(YEL "Usage: ./SPU.exe [options]\n"
           "\n"
           "Options:\n"
           "  %-16s Set name of file with input code (by default <file> = \"factorial.asm\")\n"
           "  %-16s Set name of file where compiled code should be placed (by default <file> = \"output_code\")\n"
           "\n"
           "  %-16s Assemble code without processing\n"
           "  %-16s Only process assembled code\n"
           "  %-16s Assemble and than run code\n"
           "\n"
           "  %-16s Print hint with program options\n",

           INPUT_FILE_CMD "=<file>",
           OUTPUT_FILE_CMD "=<file>",

           ASM_TYPE_CMD,
           PROC_TYPE_CMD,
           ASM_PROC_TYPE_CMD,

           HELP_FULL_CMD ", " HELP_SHORT_CMD);
}
