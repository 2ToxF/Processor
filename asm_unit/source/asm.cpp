/*
    WARNING:
    DON'T CHANGE THIS FILE - YOU MAY HAVE A LOT OF ERRORS AFTER A SMALL REMARK
    Honestly, I consciously used method of codogeneration that is really hard to debug and which causes that
        my code has become (maybe) very hard to read.
    If you want to change commands for assembler unit, than check file "include/commands.h" in origin
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "consts.h"
#include "input_output.h"
#include "utils.h"

// ------------------------------------------------------------------------------------------------------------

static const int   START_OUT_BUFSIZE   = 100;
static const int   REALLOC_COEF        = 2;
static const int   REALLOC_LIMIT       = 50;

static const int   MAX_CMD_LEN         = 40;

static const char* const HLT_CMD_TEXT  = "hlt";
static const char* const PUSH_CMD_TEXT = "push";
static const char* const POP_CMD_TEXT  = "pop";
static const char* const ADD_CMD_TEXT  = "add";
static const char* const SUB_CMD_TEXT  = "sub";
static const char* const DIV_CMD_TEXT  = "div";
static const char* const MUL_CMD_TEXT  = "mul";
static const char* const SQRT_CMD_TEXT = "sqrt";
static const char* const IN_CMD_TEXT   = "in";
static const char* const OUT_CMD_TEXT  = "out";
static const char* const DUMP_CMD_TEXT = "dump";
static const char* const RET_CMD_TEXT  = "ret";
static const char* const CALL_CMD_TEXT = "call";
static const char* const JMP_CMD_TEXT  = "jmp";
static const char* const JE_CMD_TEXT   = "je";
static const char* const JNE_CMD_TEXT  = "jne";
static const char* const JA_CMD_TEXT   = "ja";
static const char* const JAE_CMD_TEXT  = "jae";
static const char* const JB_CMD_TEXT   = "jb";
static const char* const JBE_CMD_TEXT  = "jbe";

static const char* const REGISTERS[NUM_OF_ACCESS_REGS] = {"AX", "BX", "CX", "DX"};

static const int   MAX_MARKS_ARR_SIZE = 20;

enum AssembleNumber
{
    FIRST_ASM = 1,
    SECOND_ASM,
};

struct Mark
{
    char name[MAX_CMD_LEN];
    int  cmd_ip;
};

struct ASMComponents
{
    char*  input_code_buf;
    char*  end_inp_buf;

    char*  output_code_buf;
    size_t outbuf_size;
    size_t outbuf_idx;

    Mark marks_arr[MAX_MARKS_ARR_SIZE];

    AssembleNumber asm_number;
    SPUCommands    cmd_type;
};

// ------------------------------------------------------------------------------------------------------------

static void      CheckBufAndRealloc(ASMComponents* my_asm);
static bool      CheckMarkNeces    (SPUCommands cmd_type);

static CodeError CodeAssembleOnce  (ASMComponents* my_asm);

static int       GetMarkCmdNum     (Mark* marks_arr, const char* mark_name);
static char      GetRegisterNum    (const char* reg_name);

static CodeError PutCmdWithArg     (ASMComponents* my_asm);
static CodeError PutCmdWithMark    (ASMComponents* my_asm, char* mark_name);

static void      PutOneCharInBuf   (ASMComponents* my_asm, unsigned char value);
static void      PutSomeCharsInBuf (ASMComponents* my_asm, StackElem_t value);

static CodeError ScanMark          (ASMComponents* my_asm, char cmd[MAX_CMD_LEN],
                                    size_t cmd_len, int* marks_number);

// ------------------------------------------------------------------------------------------------------------


static void CheckBufAndRealloc(ASMComponents* my_asm)
{
    if (my_asm->outbuf_idx > my_asm->outbuf_size - REALLOC_LIMIT)
    {
        my_asm->outbuf_size *= REALLOC_COEF;
        my_asm->output_code_buf = (char*) MyRecalloc(my_asm->output_code_buf, my_asm->outbuf_size, my_asm->outbuf_idx);
    }
}


static bool CheckMarkNeces(SPUCommands cmd_type)
{
    return cmd_type == CMD_JMP || cmd_type == CMD_JE || cmd_type == CMD_JNE || cmd_type == CMD_JA   ||
           cmd_type == CMD_JAE || cmd_type == CMD_JB || cmd_type == CMD_JBE || cmd_type == CMD_CALL;
}


CodeError CodeMainAssemble(const char* input_file_name, const char* output_file_name)
{
    CodeError code_err = NO_ERROR;
    ASMComponents my_asm = {};

    int input_code_bufsize = 0;
    if ((code_err = MyFread(&my_asm.input_code_buf, &input_code_bufsize, input_file_name)) != NO_ERROR)
        return code_err;

    char*  start_inp_buf    = my_asm.input_code_buf;
    my_asm.end_inp_buf      = my_asm.input_code_buf + input_code_bufsize;

    my_asm.outbuf_size      = START_OUT_BUFSIZE;
    my_asm.output_code_buf  = (char*) calloc(my_asm.outbuf_size, sizeof(char));

    my_asm.asm_number = FIRST_ASM;
    if ((code_err = CodeAssembleOnce(&my_asm)) != NO_ERROR)
        return code_err;

    my_asm.input_code_buf = start_inp_buf;
    my_asm.outbuf_idx = 0;
    my_asm.asm_number = SECOND_ASM;

    if ((code_err = CodeAssembleOnce(&my_asm)) != NO_ERROR)
        return code_err;

    FILE* out_fptr = fopen(output_file_name, "wb");
    if (out_fptr == NULL)
        return FILE_NOT_OPENED_ERR;

    fwrite(my_asm.output_code_buf, 1, my_asm.outbuf_idx, out_fptr);
    fclose(out_fptr); out_fptr = NULL;

    free(start_inp_buf);          start_inp_buf = NULL;
    free(my_asm.output_code_buf); my_asm.output_code_buf = NULL;

    return code_err;
}


#define DEF_CMD_(cmd_name, args_num, ...)                                                \
    if (strcmp(cmd, cmd_name##_CMD_TEXT) == 0)                                           \
    {                                                                                    \
        my_asm->cmd_type = CMD_##cmd_name;                                               \
                                                                                         \
        if (args_num == 0)                                                               \
        {                                                                                \
            my_asm->output_code_buf[my_asm->outbuf_idx] = (char) CMD_##cmd_name;         \
            ++my_asm->outbuf_idx;                                                        \
        }                                                                                \
                                                                                         \
        else                                                                             \
        {                                                                                \
            if ((code_err = PutCmdWithArg(my_asm)) != NO_ERROR)                          \
                return code_err;                                                         \
        }                                                                                \
    } else

static CodeError CodeAssembleOnce(ASMComponents* my_asm)
{
    CodeError code_err = NO_ERROR;

    char cmd[MAX_CMD_LEN] = {};
    int  marks_number     = 0;

    while (my_asm->input_code_buf < my_asm->end_inp_buf && sscanf(my_asm->input_code_buf, "%s", cmd) != 0)
    {
        if (cmd[0] != ';')
        {
            size_t cmd_len = strlen(cmd);

            if (cmd[cmd_len - 1] == ':')
            {
                if ((code_err = ScanMark(my_asm, cmd, cmd_len, &marks_number)) != NO_ERROR)
                    return code_err;

                BufNextString(&my_asm->input_code_buf);
                continue;
            }

            CheckBufAndRealloc(my_asm);

            #include "commands.h"

            /*else*/
            {
                printf(RED "ERROR: Meet unknown command during assembling: %s" WHT "\n", cmd);
                return UNKNOWN_CMD_ASM_ERR;
            }
        }

        BufNextString(&my_asm->input_code_buf);
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


static void PutOneCharInBuf(ASMComponents* my_asm, unsigned char value)
{
    my_asm->output_code_buf[my_asm->outbuf_idx] = value;
    ++(my_asm->outbuf_idx);
}


static CodeError PutCmdWithArg(ASMComponents* my_asm)
{
    char        arg[MAX_CMD_LEN]         = {};
    char        add_arg_str[MAX_CMD_LEN] = {};

    StackElem_t add_arg_num              = 0;
    char        register_number          = 0;

    BufNextWord(&my_asm->input_code_buf);
    sscanf(my_asm->input_code_buf, "%s", arg);

    if (StrIsNum(arg) && my_asm->cmd_type == CMD_PUSH)
    {
        PutOneCharInBuf(my_asm, ((unsigned char) my_asm->cmd_type) | IMM_T_BITMASK);
        PutSomeCharsInBuf(my_asm, (StackElem_t) atof(arg));
    }

    else if ((register_number = GetRegisterNum(arg)) != 0 &&
             (my_asm->cmd_type == CMD_POP || my_asm->cmd_type == CMD_PUSH))
    {
        PutOneCharInBuf(my_asm, ((unsigned char) my_asm->cmd_type) | REG_T_BITMASK);
        PutOneCharInBuf(my_asm, register_number);
    }

    else if (arg[0] == '[' && arg[strlen(arg)-1] == ']' &&
             (my_asm->cmd_type == CMD_POP || my_asm->cmd_type == CMD_PUSH))
    {
        sscanf(arg, "[%[^]]]", add_arg_str);

        if (StrIsNum(add_arg_str))
        {
            PutOneCharInBuf(my_asm, (unsigned char) (my_asm->cmd_type | MEM_T_BITMASK | IMM_T_BITMASK));
            PutSomeCharsInBuf(my_asm, (StackElem_t) atof(add_arg_str));
        }

        else if (sscanf(arg, "[%[^]]]", add_arg_str) == 1 &&
                 (register_number = GetRegisterNum(add_arg_str)) != 0)
        {
            PutOneCharInBuf(my_asm, (unsigned char) (my_asm->cmd_type | MEM_T_BITMASK | REG_T_BITMASK));
            PutOneCharInBuf(my_asm, register_number);
        }

        else if ((sscanf(arg, "[%lf+%[^]]]", &add_arg_num, add_arg_str) == 2 ||
                 sscanf(arg, "[%[^+]+%lf]", add_arg_str, &add_arg_num) == 2) &&
                 (register_number = GetRegisterNum(add_arg_str)) != 0)
        {
            PutOneCharInBuf(my_asm, (unsigned char) (my_asm->cmd_type | MEM_T_BITMASK | REG_T_BITMASK | IMM_T_BITMASK));
            PutSomeCharsInBuf(my_asm, add_arg_num);
            PutOneCharInBuf(my_asm, register_number);
        }
    }

    else if (CheckMarkNeces(my_asm->cmd_type))
        return PutCmdWithMark(my_asm, arg);

    else
        return UNKNOWN_ARG_ASM_ERR;

    return NO_ERROR;
}


static CodeError PutCmdWithMark(ASMComponents* my_asm, char* mark_name)
{
    if (my_asm->asm_number == SECOND_ASM)
    {
        PutOneCharInBuf(my_asm, (unsigned char) my_asm->cmd_type);

        int cmd_ip = 0;
        if ((cmd_ip = GetMarkCmdNum(my_asm->marks_arr, mark_name)) == -1)
        {
            printf(RED "ERROR: Meet undefined mark during assembling: %s" WHT "\n", mark_name);
            return UNKNOWN_MARK_NAME_ERR;
        }

        *(int*) &my_asm->output_code_buf[my_asm->outbuf_idx] = cmd_ip;
        my_asm->outbuf_idx += sizeof(int);
    }

    else
        my_asm->outbuf_idx += sizeof(int) + sizeof(char);

    return NO_ERROR;
}


static void PutSomeCharsInBuf(ASMComponents* my_asm, StackElem_t value)
{
    *(StackElem_t*) &my_asm->output_code_buf[my_asm->outbuf_idx] = value;
    my_asm->outbuf_idx += sizeof(StackElem_t);
}


static CodeError ScanMark(ASMComponents* my_asm, char cmd[MAX_CMD_LEN],
                          size_t cmd_len, int* marks_number)
{
    if (my_asm->asm_number == FIRST_ASM)
    {
        cmd[cmd_len - 1] = '\0';

        if (*marks_number >= MAX_MARKS_ARR_SIZE)
            return TOO_MUCH_MARKS_ERR;

        strcpy(my_asm->marks_arr[*marks_number].name, cmd);
        my_asm->marks_arr[*marks_number].cmd_ip = (int) my_asm->outbuf_idx;

        ++(*marks_number);
    }

    return NO_ERROR;
}
