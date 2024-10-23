/*!
    \file
    FIle with one macros which defines debug mode
*/

#ifndef DEBUG_MODE_H
#define DEBUG_MODE_H

/// @brief Debug mode (if macro is defined then debug-mode off)
// #define NDEBUG

/// @brief If defined no canaries will be set up
// #define NCANARIES_MODE

/// @brief If defined no hash will be used
// #define NHASH_MODE

/// @brief Type of stack elements
typedef double StackElem_t;

enum CodeError
{
    NO_ERROR,

    FILE_NOT_OPENED_ERR,
    FILLING_FSTAT_ERR,
    WRONG_BUFSIZE_ERR,

    OUT_OF_MEM_ERR,
    TOO_MUCH_MARKS_ERR,
    UNKNOWN_ASM_CMD_ERR,
    UNKNOWN_MARK_NAME_ERR,
    UNKNOWN_REG_NAME_ERR,

    STACK_ERR,
    UNKNOWN_RUNTIME_CMD_ERR,
};

#endif
