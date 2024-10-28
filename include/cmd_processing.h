#ifndef CMD_PROCESSING_H
#define CMD_PROCESSING_H

#include "settings.h"

#define INPUT_FILE_CMD    "--finitial"
#define OUTPUT_FILE_CMD   "--fassembled"

#define ASM_TYPE_CMD      "--asm"
#define PROC_TYPE_CMD     "--proc"
#define ASM_PROC_TYPE_CMD "--all"

#define HELP_FULL_CMD     "--help"
#define HELP_SHORT_CMD    "-h"

enum AsmProcType
{
    ONLY_ASM,
    ONLY_PROC,
    ASM_AND_PROC,
};

CodeError ReadOptions(int argc, char* argv[], char* input_file_name,
                      char* output_file_name, AsmProcType* asm_proc_type_ptr);

#endif
