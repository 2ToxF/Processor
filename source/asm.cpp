#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "global_consts.h"
#include "input_output.h"
#include "utils.h"

// ------------------------------------------------------------------------------------------------------------

static const int   START_OUT_BUFSIZE = 100;
static const int   REALLOC_COEF      = 2;
static const int   REALLOC_LIMIT     = 50;

static const int   MAX_CMD_LEN   = 40;

static const char* HLT_CMD_TEXT  = "hlt";
static const char* PUSH_CMD_TEXT = "push";
static const char* POP_CMD_TEXT  = "pop";
static const char* ADD_CMD_TEXT  = "add";
static const char* SUB_CMD_TEXT  = "sub";
static const char* DIV_CMD_TEXT  = "div";
static const char* MUL_CMD_TEXT  = "mul";
static const char* SQRT_CMD_TEXT = "sqrt";
static const char* IN_CMD_TEXT   = "in";
static const char* OUT_CMD_TEXT  = "out";
static const char* RET_CMD_TEXT  = "ret";
static const char* CALL_CMD_TEXT = "call";
static const char* JMP_CMD_TEXT  = "jmp";
static const char* JE_CMD_TEXT   = "je";
static const char* JNE_CMD_TEXT  = "jne";
static const char* JA_CMD_TEXT   = "ja";
static const char* JAE_CMD_TEXT  = "jae";
static const char* JB_CMD_TEXT   = "jb";
static const char* JBE_CMD_TEXT  = "jbe";

static const char* REGISTERS[NUM_OF_ACCESS_REGS] = {"AX", "BX", "CX", "DX"};

static const int   MAX_MARKS_ARR_SIZE = 10;

enum AssembleType
{
    ASM_NO_MARKS,
    ASM_WITH_MARKS,
};

struct Mark
{
    char name[MAX_CMD_LEN];
    int  cmd_ip;
};

// ------------------------------------------------------------------------------------------------------------

static bool      CheckMarkNeces  (SPUCommands cmd_type);
static CodeError CodeDiffAssemble(char* input_code_buf, const char* end_inp_buf,
                                  char** output_code_buf, size_t* outbuf_size, size_t* outbuf_idx,
                                  Mark marks_arr[MAX_MARKS_ARR_SIZE], AssembleType asm_type);
static int       GetMarkCmdNum   (Mark* marks_arr, const char* mark_name);
static char      GetRegisterNum  (const char* reg_name);
static CodeError PutCmdWithArg   (char** input_code_buf, char* output_code_buf,
                                  size_t* outbuf_idx, Mark marks_arr[MAX_MARKS_ARR_SIZE],
                                  SPUCommands cmd_type, AssembleType asm_type);
static CodeError PutCmdWithMark  (char* mark_name, char* output_code_buf,
                                  size_t* outbuf_idx, Mark marks_arr[MAX_MARKS_ARR_SIZE],
                                  SPUCommands cmd_type, AssembleType asm_type);

// ------------------------------------------------------------------------------------------------------------

static bool CheckMarkNeces(SPUCommands cmd_type)
{
    return cmd_type == CMD_JMP || cmd_type == CMD_JE || cmd_type == CMD_JNE || cmd_type == CMD_JA   ||
           cmd_type == CMD_JAE || cmd_type == CMD_JB || cmd_type == CMD_JBE || cmd_type == CMD_CALL;
}


CodeError CodeAssemble(const char* input_file_name, const char* output_file_name)
{
    CodeError code_err = NO_ERROR;

    char* input_code_buf     = NULL;
    int   input_code_bufsize = 0;
    if ((code_err = MyFread(&input_code_buf, &input_code_bufsize, input_file_name)) != NO_ERROR)
        return code_err;

    FILE* out_fptr = fopen(output_file_name, "wb");
    if (out_fptr == NULL)
        return FILE_NOT_OPENED_ERR;

    Mark marks_arr[MAX_MARKS_ARR_SIZE] = {};

    char*  start_inp_buf    = input_code_buf;
    char*  end_inp_buf      = input_code_buf + input_code_bufsize;

    size_t outbuf_size      = START_OUT_BUFSIZE;
    char*  output_code_buf  = (char*) calloc(outbuf_size, sizeof(char));
    size_t outbuf_idx       = 0;

    if ((code_err = CodeDiffAssemble(input_code_buf, end_inp_buf, &output_code_buf,
                                     &outbuf_size, &outbuf_idx, marks_arr, ASM_NO_MARKS)) != NO_ERROR)
        return code_err;

    outbuf_idx = 0;

    if ((code_err = CodeDiffAssemble(input_code_buf, end_inp_buf, &output_code_buf,
                                     &outbuf_size, &outbuf_idx, marks_arr, ASM_WITH_MARKS)) != NO_ERROR)
        return code_err;

    fwrite(output_code_buf, 1, outbuf_idx, out_fptr);

    free(start_inp_buf);   start_inp_buf = NULL;
    free(output_code_buf); output_code_buf = NULL;

    fclose(out_fptr); out_fptr = NULL;

    return code_err;
}


#define DEF_CMD_(cmd_name, cmd_num, args_num, ...)                                       \
    if (strcmp(cmd, cmd_name##_CMD_TEXT) == 0)                                           \
        if (args_num == 0)                                                               \
        {                                                                                \
            (*output_code_buf)[*outbuf_idx] = (char) CMD_##cmd_name;                     \
            ++(*outbuf_idx);                                                             \
        }                                                                                \
                                                                                         \
        else                                                                             \
        {                                                                                \
            if ((code_err = PutCmdWithArg(&input_code_buf, *output_code_buf,             \
                                          outbuf_idx, marks_arr,                         \
                                          CMD_##cmd_name, asm_type)) != NO_ERROR)        \
                return code_err;                                                         \
        }                                                                                \
    else

static CodeError CodeDiffAssemble(char* input_code_buf, const char* end_inp_buf,
                                  char** output_code_buf, size_t* outbuf_size, size_t* outbuf_idx,
                                  Mark marks_arr[MAX_MARKS_ARR_SIZE], AssembleType asm_type)
{
    CodeError code_err = NO_ERROR;

    char cmd[MAX_CMD_LEN] = {};
    int  marks_number     = 0;

    while (input_code_buf < end_inp_buf && sscanf(input_code_buf, "%s", cmd) != 0)
    {
        if (cmd[0] != ';')
        {
            size_t cmd_len = strlen(cmd);

            if (cmd[cmd_len - 1] == ':')
            {
                if (asm_type == ASM_NO_MARKS)
                {
                    cmd[cmd_len - 1] = '\0';

                    if (marks_number >= MAX_MARKS_ARR_SIZE)
                        return TOO_MUCH_MARKS_ERR;

                    strcpy(marks_arr[marks_number].name, cmd);
                    marks_arr[marks_number].cmd_ip = (int) *outbuf_idx;

                    ++marks_number;
                }

                BufNextString(&input_code_buf);
                continue;
            }

            if (*outbuf_idx > *outbuf_size - REALLOC_LIMIT)
            {
                *outbuf_size *= REALLOC_COEF;
                *output_code_buf = (char*) MyRecalloc(*output_code_buf, *outbuf_size, *outbuf_idx);
            }

            #include "commands.h"

            /*else*/
            {
                printf(RED "ERROR: Meet unknown command during assembling: %s" WHT "\n", cmd);
                return UNKNOWN_CMD_ASM_ERR;
            }

            cmd[0] = ';';
        }

        BufNextString(&input_code_buf);
    }

    return NO_ERROR;
}

#undef DEF_CMD_


static int GetMarkCmdNum(Mark* marks_arr, const char* mark_name)
{
    for (int i = 0; i < MAX_MARKS_ARR_SIZE; ++i)
        if (strcmp(marks_arr[i].name, mark_name) == 0)
            return marks_arr[i].cmd_ip;

    return -1;
}


static char GetRegisterNum(const char* reg_name)
{
    for (int i = 0; i < NUM_OF_ACCESS_REGS; ++i)
        if (strcmp(reg_name, REGISTERS[i]) == 0)
            return (char) (i+1);

    return 0;
}


#define PUT_1CHAR_IN_BUF_(expression)             \
    output_code_buf[*outbuf_idx] = (expression);  \
    ++(*outbuf_idx)

#define PUT_STACK_CHARS_IN_BUF_(expression)                                      \
    *(StackElem_t*) &output_code_buf[*outbuf_idx] = (StackElem_t) (expression);  \
    *outbuf_idx += sizeof(StackElem_t)

static CodeError PutCmdWithArg(char** input_code_buf, char* output_code_buf,
                               size_t* outbuf_idx, Mark marks_arr[MAX_MARKS_ARR_SIZE],
                               SPUCommands cmd_type, AssembleType asm_type)
{
    char arg[MAX_CMD_LEN] = {};
    char add_arg_str[MAX_CMD_LEN] = {};
    StackElem_t add_arg_num = 0;
    char register_number  = 0;

    BufNextWord(input_code_buf);
    sscanf(*input_code_buf, "%s", arg);

    if (StrIsNum(arg) && cmd_type == CMD_PUSH)
    {
        PUT_1CHAR_IN_BUF_(((char) cmd_type) | IMM_T_BITMASK);
        PUT_STACK_CHARS_IN_BUF_(atof(arg));
    }

    else if ((register_number = GetRegisterNum(arg)) != 0 && (cmd_type == CMD_POP || cmd_type == CMD_PUSH))
    {
        PUT_1CHAR_IN_BUF_(((char) cmd_type) | REG_T_BITMASK);
        PUT_1CHAR_IN_BUF_(register_number);
    }

    else if (arg[0] == '[' && arg[strlen(arg)-1] == ']' && (cmd_type == CMD_POP || cmd_type == CMD_PUSH))
    {
        sscanf(arg, "[%[^]]]", add_arg_str);

        if (StrIsNum(add_arg_str))
        {
            PUT_1CHAR_IN_BUF_((unsigned char) (cmd_type | MEM_T_BITMASK | IMM_T_BITMASK));
            PUT_STACK_CHARS_IN_BUF_((StackElem_t) atof(add_arg_str));
        }

        else if (sscanf(arg, "[%[^]]]", add_arg_str) == 1 &&
                 (register_number = GetRegisterNum(add_arg_str)) != 0)
        {
            PUT_1CHAR_IN_BUF_((unsigned char) (cmd_type | MEM_T_BITMASK | REG_T_BITMASK));
            PUT_1CHAR_IN_BUF_(register_number);
        }

        else if ((sscanf(arg, "[%lf+%[^]]]", &add_arg_num, add_arg_str) == 2 ||
                 sscanf(arg, "[%[^+]+%lf]", add_arg_str, &add_arg_num) == 2) &&
                 (register_number = GetRegisterNum(add_arg_str)) != 0)
        {
            PUT_1CHAR_IN_BUF_((unsigned char) (cmd_type | MEM_T_BITMASK | REG_T_BITMASK | IMM_T_BITMASK));
            PUT_STACK_CHARS_IN_BUF_(add_arg_num);
            PUT_1CHAR_IN_BUF_(register_number);
        }
    }

    else if (CheckMarkNeces(cmd_type))
        return PutCmdWithMark(arg, output_code_buf, outbuf_idx, marks_arr, cmd_type, asm_type);

    else
        return UNKNOWN_ARG_ASM_ERR;

    return NO_ERROR;
}


static CodeError PutCmdWithMark(char* mark_name, char* output_code_buf,
                                size_t* outbuf_idx, Mark marks_arr[MAX_MARKS_ARR_SIZE],
                                SPUCommands cmd_type, AssembleType asm_type)
{
    if (asm_type == ASM_WITH_MARKS)
    {
        PUT_1CHAR_IN_BUF_((unsigned char) cmd_type);

        int cmd_ip = 0;
        if ((cmd_ip = GetMarkCmdNum(marks_arr, mark_name)) == -1)
        {
            printf(RED "ERROR: Meet undefined mark during assembling: %s" WHT "\n", mark_name);
            return UNKNOWN_MARK_NAME_ERR;
        }

        *(int*) &output_code_buf[*outbuf_idx] = cmd_ip;
        *outbuf_idx += sizeof(int);
    }

    else
        *outbuf_idx += sizeof(int) + sizeof(char);

    return NO_ERROR;
}

#undef PUT_1CHAR_IN_BUF_
#undef PUT_STACK_CHARS_IN_BUF_
