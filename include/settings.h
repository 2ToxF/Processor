/*!
    \file
    FIle with one macros which defines debug mode
*/

#ifndef DEBUG_MODE_H
#define DEBUG_MODE_H

/// @brief Debug mode (if macro is defined then debug-mode off)
#define NDEBUG

/// @brief If defined no canaries will be set up
// #define NCANARIES_MODE

/// @brief If defined no hash will be used
// #define NHASH_MODE

enum CodeError
{
    NO_ERROR,
    FILE_NOT_OPENED_ERR,
    WRONG_BUFSIZE_ERR,
    FILLING_FSTAT_ERR,
};

#endif
