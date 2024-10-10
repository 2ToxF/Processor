#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "utils.h"

/// @brief Type of canaries on the stack sides
typedef uint64_t canary_t;

/// @brief Key to find real pointer using XOR
static size_t key_for_ptr_dec = 0;

//----------------------------------------------------------------------------------------------------------------------

/// @brief Default number of elements that can be put to stack
static const size_t DEFAULT_STK_CAPACITY = 8;

/// @brief Maximum length of string that is used to make dump beautiful
static const int MAX_TEMP_DUMP_STR_LEN = 12;

/// @brief Coefficeint for upsizing stack
static const int RESIZE_COEF      = 2;

/// @brief Coefficient that sets ratio of minimum and maximum number of elements (except default capacity)
static const int RESIZE_COEF_DOWN = RESIZE_COEF * 2;

/// @brief Canary value for securing stack structure
static const canary_t STACK_CANARY_VALUE = 0xBAD57ACCBAD57ACC;

/// @brief Canary value for securing stack elements
static const canary_t DATA_CANARY_VALUE  = 0xBADDA7A0BADDA7A0;

/// @brief Size of all canaries
static const size_t SIZE_OF_CANARY       = sizeof(canary_t);

//----------------------------------------------------------------------------------------------------------------------

#ifndef NCANARIES_MODE
    /// @brief Sets up canaries in stack structure and data or not depending on canaries mode
    #define CANARIES_SET_UP(...) __VA_ARGS__
#else
    /// @brief Sets up canaries in stack structure and data or not depending on canaries mode
    #define CANARIES_SET_UP(...)
#endif

#ifndef NHASH_MODE
    /// @brief Sets up hash in stack structure or not depending on hash mode
    #define HASH_SET_UP(...) __VA_ARGS__

    /// @brief Checks stack hash or not depending on debug mode
    #define STACK_HASH(stk) StackHash(stk)
#else
    /// @brief Sets up hash in stack structure depending on hash mode
    #define HASH_SET_UP(...)

    /// @brief Checks stack hash or not depending on debug mode
    #define STACK_HASH(stk)
#endif

/// @brief Sructure with stack info
struct stack_t
{
    CANARIES_SET_UP(canary_t left_canary);
    ON_DEBUG(const char* stk_name);
    ON_DEBUG(const char* init_file);
    ON_DEBUG(int init_line);
    ON_DEBUG(const char* init_func);
    HASH_SET_UP(unsigned long hash_struct);
    HASH_SET_UP(unsigned long hash_data);
    unsigned int stack_errors;
    StackElem_t* data;
    int index;
    int capacity;
    CANARIES_SET_UP(canary_t right_canary);
};

//----------------------------------------------------------------------------------------------------------------------

#ifndef NHASH_MODE
/*! -----------------------------------------------------------------------------------------------------
    Calculates both structure and data (elements) hashes of stack
    \param[in, out]  stk  Pointer to stack sructure
    \return Type of code error or 0 for "no error"-state
    ----------------------------------------------------------------------------------------------------- */
static StackError StackHash      (stack_t* stk);

/*! -----------------------------------------------------------------------------------------------------
    Calculates stack data (elements) hash using DJB2-algorithm
    \param[in, out]  stk  Pointer to stack sructure
    \return Type of code error or 0 for "no error"-state
    ----------------------------------------------------------------------------------------------------- */
static StackError StackHashData  (stack_t* stk);

/*! -----------------------------------------------------------------------------------------------------
    Calculates stack structure hash using DJB2-algorithm
    \param[in, out]  stk  Pointer to stack sructure
    \return Type of code error or 0 for "no error"-state
    ----------------------------------------------------------------------------------------------------- */
static StackError StackHashStruct(stack_t* stk);
#endif

/*! -----------------------------------------------------------------------------------------------------
    Calculates the real pointer to stack structure using XOR with key_for_decode
    \param[in]  ptr_do_decode  Encoded (decoded) pointer to stack sructure
    \return Decoded (encoded) pointer to stack structure
    ----------------------------------------------------------------------------------------------------- */
static size_t    StackPtrXOR    (size_t ptr_to_decode);

/*! -----------------------------------------------------------------------------------------------------
    Downsizes the stack
    \param[in, out]  stk  Pointer to stack sructure
    \return Type of code error or 0 for "no error"-state
    ----------------------------------------------------------------------------------------------------- */
static StackError StackResizeDown(stack_t* stk);

/*! -----------------------------------------------------------------------------------------------------
    Upsizes the stack
    \param[in, out]  stk  Pointer to stack sructure
    \return Type of code error or 0 for "no error"-state
    ----------------------------------------------------------------------------------------------------- */
static StackError StackResizeUp  (stack_t* stk);

/*!
    Verifies all stack's sructure and data (elements)
    \param[in]  stk  Pointer to stack sructure
    \return Type of code error or 0 for "no error"-state
*/
static StackError StackVerifyAll(stack_t* stk);

/*!
    Verifies critical (the most important) parts of stack's sructure and data (elements)
    \param[in]  stk  Pointer to stack sructure
    \return Type of code error or 0 for "no error"-state
*/
static StackError StackVerifyCritical(stack_t* stk);


#ifndef NDEBUG
    /*!
        Prints stack info (ONLY DEFINED IN DEBUG MODE)
        \param[in]  stk          Pointer to structure of stack that should be printed
        \param[in]  file_name    Name of file where function was called
        \param[in]  line_number  Number of line where function was called
    */
    void StackDump(const stack_t* stk, const char* file_name, int line_number);

    /// @brief Macro for verifying stack
    #define STACK_VERIFY(stk)                                    \
        do                                                       \
        {                                                        \
            StackError temp_stack_err = STK_NO_ERROR;                  \
            if ((temp_stack_err = StackVerifyAll(stk)) != STK_NO_ERROR)  \
            {                                                    \
                if (temp_stack_err >= STACK_ANTIOVERFLOW_ERR)     \
                    StackDump(stk, __FILE__, __LINE__);          \
                return temp_stack_err;                            \
            }                                                    \
        } while(0)
#else
    /// @brief Macro for verifying stack
    #define STACK_VERIFY(stk)                                    \
        do {                                                     \
            StackError temp_stack_err = STK_NO_ERROR;                  \
            if ((temp_stack_err = StackVerifyAll(stk)) != STK_NO_ERROR)  \
                return temp_stack_err;                            \
        } while(0)
#endif


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//----------------------------------------------------------------------------------------------------------------
// ------> !!! STACK PART !!! <-----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


StackError StackDtor(size_t* stk_enc_ptr)
{
    stack_t* stk = (stack_t*) StackPtrXOR(*stk_enc_ptr);

    STACK_VERIFY(stk);

    #ifndef NCANARIES_MODE
        free((char*) stk->data - SIZE_OF_CANARY); stk->data = NULL;
    #else
        free(stk->data); stk->data = NULL;
    #endif
    stk->index = 0;
    stk->capacity = 0;
    free(stk); stk = NULL;

    *stk_enc_ptr = 0;

    return STK_NO_ERROR;
}

//----------------------------------------------------------------------------------------------------------------------

#ifndef NHASH_MODE
    static StackError StackHash(stack_t* stk)
    {
        StackError stack_err = STK_NO_ERROR;
        if ((stack_err = StackVerifyCritical(stk)) != STK_NO_ERROR)
            return stack_err;

        if ((stack_err = StackHashStruct(stk)) != STK_NO_ERROR)
            return stack_err;

        if ((stack_err = StackHashData(stk)) != STK_NO_ERROR)
            return stack_err;

        return stack_err;
    }

//----------------------------------------------------------------------------------------------------------------------

    static StackError StackHashData(stack_t* stk)
    {
        StackError stack_err = STK_NO_ERROR;
        if ((stack_err = StackVerifyCritical(stk)) != STK_NO_ERROR)
            return stack_err;

        stk->hash_data = 0;
        stk->hash_data = MyHash(stk->data, stk->capacity*sizeof(StackElem_t));

        return STK_NO_ERROR;
    }

//----------------------------------------------------------------------------------------------------------------------

    static StackError StackHashStruct(stack_t* stk)
    {
        StackError stack_err = STK_NO_ERROR;
        if ((stack_err = StackVerifyCritical(stk)) != STK_NO_ERROR)
            return stack_err;

        stk->hash_struct = 0;
        unsigned long temp_hash_data = stk->hash_data;
        stk->hash_data = 0;

        stk->hash_struct = MyHash(stk, sizeof(*stk));
        stk->hash_data = temp_hash_data;

        return STK_NO_ERROR;
    }
#endif

//----------------------------------------------------------------------------------------------------------------------

DIF_STACK_INIT
{
    stack_t* stk = (stack_t*) calloc(1, sizeof(stack_t));

    #ifndef NDEBUG
        if (stk->data != NULL || stk->index != 0 || stk->capacity != 0)
        {
            StackDump(stk, __FILE__, __LINE__);
            return STACK_ALREADY_INITED_ERR;
        }

        stk->stk_name = stk_name;
        stk->init_file = stk_init_file;
        stk->init_line = stk_init_line;
        stk->init_func = stk_init_func;
    #endif

    if (key_for_ptr_dec == 0)
    {
        if ((key_for_ptr_dec = MyGetRandom64()) == 0)
            return CANT_CREATE_RAND_NUM_ERR;
    }

    #ifndef NCANARIES_MODE
        stk->left_canary = stk->right_canary = STACK_CANARY_VALUE;
    #endif

    #ifndef NCANARIES_MODE
        char* new_data = (char*) calloc(DEFAULT_STK_CAPACITY*sizeof(StackElem_t) + SIZE_OF_CANARY*2, sizeof(char));
        *((canary_t*) new_data) = DATA_CANARY_VALUE;
        *((canary_t*) (new_data + DEFAULT_STK_CAPACITY*sizeof(StackElem_t) + SIZE_OF_CANARY)) = DATA_CANARY_VALUE;
        stk->data = (StackElem_t*) (new_data + SIZE_OF_CANARY);
    #else
        stk->data = (StackElem_t*) calloc(DEFAULT_STK_CAPACITY, sizeof(StackElem_t));
    #endif

    stk->index = 0;
    stk->capacity = DEFAULT_STK_CAPACITY;

    STACK_HASH(stk);
    STACK_VERIFY(stk);

    *stk_enc_ptr = StackPtrXOR((size_t) stk);

    return STK_NO_ERROR;
}

//----------------------------------------------------------------------------------------------------------------------

StackError StackPop(size_t stk_enc_ptr, StackElem_t* var)
{
    stack_t* stk = (stack_t*) StackPtrXOR(stk_enc_ptr);

    STACK_VERIFY(stk);

    StackError stack_err = STK_NO_ERROR;
    #ifndef NDEBUG
        if (stk->index == 0)
        {
            StackDump(stk, __FILE__, __LINE__);
            return STACK_ANTIOVERFLOW_ERR;
        }
    #endif

    if (stk->index == stk->capacity / RESIZE_COEF_DOWN && (size_t) stk->capacity > DEFAULT_STK_CAPACITY)
        if ((stack_err = StackResizeDown(stk)) != STK_NO_ERROR)
            return stack_err;

    stk->index--;
    *var = stk->data[stk->index];
    stk->data[stk->index] = 0;

    STACK_HASH(stk);
    STACK_VERIFY(stk);
    return STK_NO_ERROR;
}

//----------------------------------------------------------------------------------------------------------------------

StackError StackPush(size_t stk_enc_ptr, StackElem_t value)
{
    stack_t* stk = (stack_t*) StackPtrXOR(stk_enc_ptr);

    STACK_VERIFY(stk);

    StackError stack_err = STK_NO_ERROR;
    if (stk->index == stk->capacity)
        if ((stack_err = StackResizeUp(stk)) != STK_NO_ERROR)
            return stack_err;

    stk->data[stk->index] = value;
    stk->index++;

    STACK_HASH(stk);
    STACK_VERIFY(stk);
    return STK_NO_ERROR;
}

//----------------------------------------------------------------------------------------------------------------------

static size_t StackPtrXOR(size_t ptr_to_decode)
{
    return ptr_to_decode ^ key_for_ptr_dec;
}

//----------------------------------------------------------------------------------------------------------------------

static StackError StackResizeDown(stack_t* stk)
{
    STACK_VERIFY(stk);

    stk->capacity /= RESIZE_COEF;

    #ifndef NCANARIES_MODE
        char* new_data = (char*) realloc((char*) stk->data - SIZE_OF_CANARY,
                                        stk->capacity*sizeof(StackElem_t) + SIZE_OF_CANARY*2) + SIZE_OF_CANARY;

        if (new_data == NULL)
            return OUT_OF_MEMORY_ERR;

        *((canary_t*) (new_data + stk->capacity*sizeof(StackElem_t))) = DATA_CANARY_VALUE;
    #else
        StackElem_t* new_data = (StackElem_t*) realloc(stk->data, stk->capacity*sizeof(StackElem_t));

        if (new_data == NULL)
            return OUT_OF_MEMORY_ERR;
    #endif

    stk->data = (StackElem_t*) new_data;

    STACK_HASH(stk);
    STACK_VERIFY(stk);
    return STK_NO_ERROR;
}

//----------------------------------------------------------------------------------------------------------------------

static StackError StackResizeUp(stack_t* stk)
{
    STACK_VERIFY(stk);

    stk->capacity *= RESIZE_COEF;

    #ifndef NCANARIES_MODE
        char* new_data = (char*) realloc((char*) stk->data - SIZE_OF_CANARY,
                                        stk->capacity*sizeof(StackElem_t) + SIZE_OF_CANARY*2) + SIZE_OF_CANARY;

        if (new_data == NULL)
            return OUT_OF_MEMORY_ERR;

        *((canary_t*) (new_data + stk->capacity*sizeof(StackElem_t))) = DATA_CANARY_VALUE;
    #else
        StackElem_t* new_data = (StackElem_t*) realloc(stk->data, stk->capacity*sizeof(StackElem_t));

        if (new_data == NULL)
            return OUT_OF_MEMORY_ERR;
    #endif

    memset(new_data + stk->index*sizeof(StackElem_t), 0, (stk->capacity - stk->index)*sizeof(StackElem_t));

    stk->data = (StackElem_t*) new_data;

    STACK_HASH(stk);
    STACK_VERIFY(stk);
    return STK_NO_ERROR;
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//----------------------------------------------------------------------------------------------------------------
// ------> !!! VERIFY PART !!! <----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


static StackError StackVerifyCritical(stack_t* stk)
{
    if (stk == NULL || (size_t) stk == key_for_ptr_dec)
    {
        stk->stack_errors = NULL_STK_STRUCT_PTR_ERR;
        return NULL_STK_STRUCT_PTR_ERR;
    }

    if (stk->data == NULL)
    {
        stk->stack_errors = NULL_STK_DATA_PTR_ERR;
        return NULL_STK_DATA_PTR_ERR;
    }

    if (stk->capacity < 0)
    {
        stk->stack_errors = NEG_STK_CAPACITY_ERR;
        return NEG_STK_CAPACITY_ERR;
    }

    if (stk->index < 0)
    {
        stk->stack_errors = STACK_ANTIOVERFLOW_ERR;
        return STACK_ANTIOVERFLOW_ERR;
    }

    if (stk->index > stk->capacity)
    {
        stk->stack_errors = STACK_OVERFLOW_ERR;
        return STACK_OVERFLOW_ERR;
    }

    return STK_NO_ERROR;
}


static StackError StackVerifyAll(stack_t* stk)
{
    StackError stack_err = STK_NO_ERROR;
    if ((stack_err = StackVerifyCritical(stk)) != STK_NO_ERROR)
        return stack_err;

    #ifndef NDEBUG
        if (stk->index < stk->capacity / RESIZE_COEF_DOWN - 1 && (size_t) stk->capacity > DEFAULT_STK_CAPACITY)
            stack_err = STACK_USES_MUCH_MEM_ERR;
    #endif

    #ifndef NCANARIES_MODE
        if (stk->left_canary != STACK_CANARY_VALUE || stk->right_canary != STACK_CANARY_VALUE)
            stack_err = STKSTRUCT_CANARY_CORRUPT_ERR;
        if (*((canary_t*) ((char*) stk->data - SIZE_OF_CANARY)) != DATA_CANARY_VALUE
            || *((canary_t*) (stk->data + stk->capacity)) != DATA_CANARY_VALUE)
            stack_err = STKDATA_CANARY_CORRUPT_ERR;
    #endif

    #ifndef NHASH_MODE
        unsigned long temp_hash_data = stk->hash_data, temp_hash_struct = stk->hash_struct;
        STACK_HASH(stk);
        if (temp_hash_struct != stk->hash_struct)
            stack_err = STKSTRUCT_INFO_CORRUPT_ERR;
        if (temp_hash_data != stk->hash_data)
            stack_err = STKDATA_INFO_CORRUPT_ERR;
    #endif

    return stack_err;
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//----------------------------------------------------------------------------------------------------------------
// ------> !!! INPUT/OUTPUT PART !!! <----------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


#ifndef NDEBUG
    void StackDump(const stack_t* stk, const char* file_name, int line_number)
    {
        int idx_first_of_same = 0;
        char index_string[MAX_TEMP_DUMP_STR_LEN] = {};

        #define PRINT_CELLS_VALUE_(idx)                                             \
            {                                                                       \
                if (idx_first_of_same == (idx))                                     \
                    sprintf(index_string, "*[%d]", (idx));                          \
                else                                                                \
                    sprintf(index_string, "*[%d - %d]", idx_first_of_same, (idx));  \
                printf("\t\t%-12s  =  %d\n", index_string, stk->data[idx]);         \
                idx_first_of_same = (idx + 1);                                      \
            }


        printf(RED "stack_t " MAG "%s [0x%p] " BLU "at %s:%d " YEL "born at %s:%d (%s)\n",
            stk->stk_name, stk,
            file_name, line_number,
            stk->init_file, stk->init_line, stk->init_func);
        printf(GRN "{\n"
            "%-10s= %d\n"
            "%-10s= %d\n"
            "\n"
            MAG "\tdata [0x%p]:\n"
            CYN "\t{\n",
            "\tindex", stk->index,
            "\tcapacity", stk->capacity,
            stk->data);

        for (int i = 1; i < stk->capacity; i++)
        {
            if (stk->data[i] != stk->data[i-1])
                PRINT_CELLS_VALUE_(i-1);
        }
        PRINT_CELLS_VALUE_(stk->capacity-1);

        printf("\t}\n"
            GRN "}\n" WHT);

        #undef PRINT_CELLS_VALUE
    }
#endif
