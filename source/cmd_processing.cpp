#include <stdio.h>
#include <string.h>

#include "cmd_processing.h"
#include "input_output.h"

static const int MAX_OPTION_LEN = 25;


CodeError ReadOptions(int argc, char* argv[], char* input_file_name,
                      char* output_file_name, AsmProcType* asm_proc_type_ptr)
{
    for (int i = 1; i < argc; i++)
    {
        char option_part1[MAX_OPTION_LEN] = {}, option_part2[MAX_OPTION_LEN] = {};

        if (sscanf(argv[i], "%[^=]=%s", option_part1, option_part2) == 2)
        {
            if (strcmp(option_part1, INPUT_FILE_CMD) == 0)
                strcpy(input_file_name, option_part2);

            else if (strcmp(option_part1, OUTPUT_FILE_CMD) == 0)
                strcpy(output_file_name, option_part2);

            else
            {
                PrintHelp();
                return UNKNOWN_OPTION_ERR;
            }
        }

        else
        {
            if (strcmp(argv[i], ASM_TYPE_CMD) == 0)
                *asm_proc_type_ptr = ONLY_ASM;

            else if (strcmp(argv[i], PROC_TYPE_CMD) == 0)
                *asm_proc_type_ptr = ONLY_PROC;

            else if (strcmp(argv[i], ASM_PROC_TYPE_CMD) == 0)
                *asm_proc_type_ptr = ASM_AND_PROC;

            else if (strcmp(argv[i], HELP_FULL_CMD) == 0 || strcmp(argv[i], HELP_SHORT_CMD) == 0)
            {
                PrintHelp();
                return CMD_HELP_NO_ERR;
            }

            else
            {
                PrintHelp();
                return UNKNOWN_OPTION_ERR;
            }
        }
    }

    return NO_ERROR;
}
