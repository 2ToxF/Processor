/*!
    \file
    FIle with things which defines main settings of the program
*/

#ifndef DEBUG_MODE_H
#define DEBUG_MODE_H

/// @brief Debug mode (if macro is defined then debug-mode off)
// #define NDEBUG

/// @brief If defined no canaries will be set up
// #define NCANARIES_MODE

/// @brief If defined no hash will be used
// #define NHASH_MODE

const char* const DEFAULT_INPUT_FILE_NAME  = "asm_files/factorial.asm";
const char* const DEFAULT_OUTPUT_FILE_NAME = "output_code";

/// @brief Type of stack elements
typedef double StackElem_t;

enum CodeError
{
    CMD_HELP_NO_ERR = -1,
    NO_ERROR,

    UNKNOWN_OPTION_ERR,

    FILE_NOT_OPENED_ERR,
    FILLING_FSTAT_ERR,
    WRONG_BUFSIZE_ERR,

    OUT_OF_MEM_ERR,
    TOO_MUCH_MARKS_ERR,
    UNKNOWN_ARG_ASM_ERR,
    UNKNOWN_CMD_ASM_ERR,
    UNKNOWN_MARK_NAME_ERR,
    UNKNOWN_REG_NAME_ERR,

    STACK_ERR,
    UNKNOWN_RUNTIME_CMD_ERR,
};

#endif
