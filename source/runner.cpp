#include "stdio.h"
#include "string.h"

#include "asm.h"
#include "cmd_processing.h"
#include "input_output.h"
#include "spu.h"
#include "runner.h"

static const char* const DEFAULT_INPUT_FILE_NAME  = "factorial.asm";
static const char* const DEFAULT_OUTPUT_FILE_NAME = "output_code";

static const int MAX_FILE_NAME_LEN = 50;


CodeError RunMainProgram(int argc, char* argv[])
{
    CodeError code_err = NO_ERROR;

    char input_file_name[MAX_FILE_NAME_LEN] = {};
    strcpy(input_file_name, DEFAULT_INPUT_FILE_NAME);

    char output_file_name[MAX_FILE_NAME_LEN] = {};
    strcpy(output_file_name, DEFAULT_OUTPUT_FILE_NAME);

    AsmProcType asm_proc_type = ASM_AND_PROC;

    if ((code_err = ReadOptions(argc, argv, input_file_name, output_file_name, &asm_proc_type)) != NO_ERROR)
        return code_err;

    if (asm_proc_type == ONLY_ASM || asm_proc_type == ASM_AND_PROC)
    {
        if ((code_err = CodeAssemble(input_file_name, output_file_name)) != NO_ERROR)
            return code_err;

        printf(MAG "Program assembled succesfully" WHT "\n");
    }

    if (asm_proc_type == ONLY_PROC || asm_proc_type == ASM_AND_PROC)
    {
        int user_answer = 0;
        printf(MAG "Wanna run your code? (Y / N)" WHT "\n");

        if (((user_answer = getchar()) == 'Y' || user_answer == 'y' ||
              user_answer == 'N' || user_answer == 'n') & (getchar() == '\n'))
        {
            if (user_answer == 'Y' || user_answer == 'y')
            {
                printf(MAG "Starting program" WHT "\n");
                code_err = RunCode(output_file_name);
            }

            else
                printf(MAG "Stopping program" WHT "\n");
        }

        else
            printf(RED "Sorry, I don't know such command" WHT "\n");
    }

    return code_err;
}
