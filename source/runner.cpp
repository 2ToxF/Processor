#include "stdio.h"

#include "asm.h"
#include "input_output.h"
#include "spu.h"
#include "runner.h"


CodeError RunMainProgram()
{
    CodeError code_err = CodeAssemble(INPUT_CODE_FILE_NAME, OUTPUT_CODE_FILE_NAME);
    if (code_err != NO_ERROR)
        return code_err;

    int user_answer = 0;
    printf(MAG "Program assembled succesfully\nWanna run it? (Y / N)" WHT "\n");

    if (((user_answer = getchar()) == 'Y' || user_answer == 'y' || user_answer == 'N'
          || user_answer == 'n') & (getchar() == '\n'))
    {
        if (user_answer == 'Y' || user_answer == 'y')
        {
            printf(MAG "Starting program" WHT "\n");
            code_err = RunCode(OUTPUT_CODE_FILE_NAME);
        }

        else
            printf(MAG "Stopping program" WHT "\n");
    }

    else
        printf(RED "Sorry, I don't know such command" WHT "\n");

    return code_err;
}
