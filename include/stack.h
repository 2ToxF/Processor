/*!
    \file
    File with constructions for main work with stack
*/

#ifndef STACK_H
#define STACK_H

#include "settings.h"

#ifndef NDEBUG
    /// @brief Differently defined StackInit() function depending on debug mode
    #define DIF_STACK_INIT                                                                         \
        StackError StackInit(size_t* stk_enc_ptr, const char* stk_name, const char* stk_init_file,  \
                            int stk_init_line, const char* stk_init_func)                          \

    /// @brief Is replaced with it's arguements only in debug mode
    #define ON_DEBUG(...) __VA_ARGS__

    /// @brief Creates stack structure depending on debug mode
    #define CREATE_STACK(stk_enc_ptr) StackInit(stk_enc_ptr, #stk_enc_ptr, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#else
    /// @brief Differently defined StackInit() function depending on debug mode
    #define DIF_STACK_INIT StackError StackInit(size_t* stk_enc_ptr)

    /// @brief Is replaced with it's arguements only in debug mode
    #define ON_DEBUG(...)

    /// @brief Creates stack structure depending on debug mode
    #define CREATE_STACK(stk_enc_ptr) StackInit(stk_enc_ptr)
#endif

/// @brief Type of stack elements
typedef int StackElem_t;

/// @brief Enumerated types of code errors or 0 for "no error"-state associated with stack
enum StackError
{
    STK_NO_ERROR                  =  0u,
    CANT_CREATE_RAND_NUM_ERR      =  1u,
    STACK_ALREADY_INITED_ERR      =  2u,
    NULL_STK_STRUCT_PTR_ERR       =  4u,
    NULL_STK_DATA_PTR_ERR         =  8u,
    NEG_STK_CAPACITY_ERR          =  16u,
    STACK_ANTIOVERFLOW_ERR        =  32u,
    STACK_OVERFLOW_ERR            =  64u,
    OUT_OF_MEMORY_ERR             =  128u,
    STACK_USES_MUCH_MEM_ERR       =  256u,
    STKSTRUCT_CANARY_CORRUPT_ERR  =  512u,
    STKDATA_CANARY_CORRUPT_ERR    =  1024u,
    STKSTRUCT_INFO_CORRUPT_ERR    =  2048u,
    STKDATA_INFO_CORRUPT_ERR      =  4096u,
};

/// @brief Sructure with stack info
struct stack_t;

/*! -----------------------------------------------------------------------------------------------------
    Destruct stack
    \param[in, out]  stk_enc_ptr  Encoded pointer to stack sructure
    ----------------------------------------------------------------------------------------------------- */
StackError StackDtor      (size_t* stk_enc_ptr);

/*! -----------------------------------------------------------------------------------------------------
    Extractes value from stack
    \param[in]   stk_enc_ptr  Encoded pointer to stack sructure
    \param[out]  var          Pointer to variable where extracted value should be put
    \return Type of code error or 0 for "no error"-state
    ----------------------------------------------------------------------------------------------------- */
StackError StackPop       (size_t stk_enc_ptr, StackElem_t* var);

/*! -----------------------------------------------------------------------------------------------------
    Extractes value from stack
    \param[in]  stk_enc_ptr Encoded pointer to stack sructure
    \param[in]  value       Value that should be put to stack
    \return Type of code error or 0 for "no error"-state
    ----------------------------------------------------------------------------------------------------- */
StackError StackPush      (size_t stk_enc_ptr, StackElem_t value);

DIF_STACK_INIT;

#endif
