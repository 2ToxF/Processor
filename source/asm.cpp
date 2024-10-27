#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "commands.h"
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
    int  cmd_num;
};

// ------------------------------------------------------------------------------------------------------------

static bool      CheckMarkNeces  (SPUCommands cmd_type);
static CodeError CodeDiffAssemble(char* input_code_buf, const char* end_inp_buf,
                                  char** output_code_buf, size_t* outbuf_size, size_t* outbuf_idx,
                                  Mark marks_arr[MAX_MARKS_ARR_SIZE], AssembleType asm_type);
static CodeError GetCmdWithArg   (char** input_code_buf, char* output_code_buf,
                                  size_t* outbuf_idx, Mark marks_arr[MAX_MARKS_ARR_SIZE],
                                  SPUCommands cmd_type, AssembleType asm_type);
static int       GetMarkCmdNum   (Mark* marks_arr, const char* mark_name);
static char      GetRegisterNum  (const char* reg_name);

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
                    marks_arr[marks_number].cmd_num = (int) *outbuf_idx;

                    ++marks_number;

                    // for (int i = 0; i < MAX_MARKS_ARR_SIZE; ++i)
                    //     printf("mark '%s': %d\n", marks_arr[i].name, marks_arr[i].cmd_num);
                }

                BufNextString(&input_code_buf);
                continue;
            }

            if (*outbuf_idx > *outbuf_size - REALLOC_LIMIT)
            {
                *outbuf_size *= REALLOC_COEF;
                *output_code_buf = (char*) MyRecalloc(*output_code_buf, *outbuf_size, *outbuf_idx);
            }

            if (strcmp(cmd, HLT_CMD_TEXT) == 0)
            {
                (*output_code_buf)[*outbuf_idx] = (char) CMD_HLT;
                ++(*outbuf_idx);
            }

            else if (strcmp(cmd, PUSH_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_PUSH, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else if (strcmp(cmd, POP_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_POP, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else if (strcmp(cmd, ADD_CMD_TEXT) == 0)  // TODO: delete copypaste
            {
                (*output_code_buf)[*outbuf_idx] = (char) CMD_ADD;
                ++(*outbuf_idx);
            }

            else if (strcmp(cmd, SUB_CMD_TEXT) == 0)
            {
                (*output_code_buf)[*outbuf_idx] = (char) CMD_SUB;
                ++(*outbuf_idx);
            }

            else if (strcmp(cmd, DIV_CMD_TEXT) == 0)
            {
                (*output_code_buf)[*outbuf_idx] = (char) CMD_DIV;
                ++(*outbuf_idx);
            }

            else if (strcmp(cmd, MUL_CMD_TEXT) == 0)
            {
                (*output_code_buf)[*outbuf_idx] = (char) CMD_MUL;
                ++(*outbuf_idx);
            }

            else if (strcmp(cmd, SQRT_CMD_TEXT) == 0)
            {
                (*output_code_buf)[*outbuf_idx] = (char) CMD_SQRT;
                ++(*outbuf_idx);
            }

            else if (strcmp(cmd, IN_CMD_TEXT) == 0)
            {
                (*output_code_buf)[*outbuf_idx] = (char) CMD_IN;
                ++(*outbuf_idx);
            }

            else if (strcmp(cmd, OUT_CMD_TEXT) == 0)
            {
                (*output_code_buf)[*outbuf_idx] = (char) CMD_OUT;
                ++(*outbuf_idx);
            }

            else if (strcmp(cmd, RET_CMD_TEXT) == 0)
            {
                (*output_code_buf)[*outbuf_idx] = (char) CMD_RET;
                ++(*outbuf_idx);
            }

            else if (strcmp(cmd, JMP_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_JMP, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else if (strcmp(cmd, JE_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_JE, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else if (strcmp(cmd, JNE_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_JNE, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else if (strcmp(cmd, JA_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_JA, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else if (strcmp(cmd, JAE_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_JAE, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else if (strcmp(cmd, JB_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_JB, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else if (strcmp(cmd, JBE_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_JBE, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else if (strcmp(cmd, CALL_CMD_TEXT) == 0)
            {
                if ((code_err = GetCmdWithArg(&input_code_buf, *output_code_buf,
                                              outbuf_idx, marks_arr, CMD_CALL, asm_type)) != NO_ERROR)
                    return code_err;
            }

            else
            {
                printf(RED "ERROR: Meet unknown command during assembling: %s" WHT "\n", cmd);
                return UNKNOWN_ASM_CMD_ERR;
            }

            cmd[0] = ';';
        }

        BufNextString(&input_code_buf);
    }

    return NO_ERROR;
}


static CodeError GetCmdWithArg(char** input_code_buf, char* output_code_buf,
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
        output_code_buf[*outbuf_idx] = ((char) cmd_type) | IMM_T_BITMASK;
        ++(*outbuf_idx);

        *(StackElem_t*) &output_code_buf[*outbuf_idx] = (StackElem_t) atof(arg);
        *outbuf_idx += sizeof(StackElem_t);
    }

    else if ((register_number = GetRegisterNum(arg)) != 0 && (cmd_type == CMD_POP || cmd_type == CMD_PUSH))
    {
        output_code_buf[*outbuf_idx] = ((char) cmd_type) | REG_T_BITMASK;
        ++(*outbuf_idx);

        output_code_buf[*outbuf_idx] = register_number;
        ++(*outbuf_idx);
    }

    else if (arg[0] == '[' && arg[strlen(arg)-1] == ']' && (cmd_type == CMD_POP || cmd_type == CMD_PUSH))
    {
        sscanf(arg, "[%[^]]]", add_arg_str);

        if (StrIsNum(add_arg_str))
        {
            output_code_buf[*outbuf_idx] = (unsigned char) (cmd_type | MEM_T_BITMASK | IMM_T_BITMASK);
            ++(*outbuf_idx);

            *(StackElem_t*) &output_code_buf[*outbuf_idx] = (StackElem_t) atof(add_arg_str);
            *outbuf_idx += sizeof(StackElem_t);
        }

        else if (sscanf(arg, "[%[^]]]", add_arg_str) == 1 &&
                 (register_number = GetRegisterNum(add_arg_str)) != 0)
        {
            output_code_buf[*outbuf_idx] = (unsigned char) (cmd_type | MEM_T_BITMASK | REG_T_BITMASK);
            ++(*outbuf_idx);

            output_code_buf[*outbuf_idx] = register_number;
            ++(*outbuf_idx);
        }

        else if ((sscanf(arg, "[%lf+%[^]]]", &add_arg_num, add_arg_str) == 2 ||
                 sscanf(arg, "[%[^+]+%lf]", add_arg_str, &add_arg_num) == 2) &&
                 (register_number = GetRegisterNum(add_arg_str)) != 0)
        {
            output_code_buf[*outbuf_idx] = (unsigned char) (cmd_type | MEM_T_BITMASK | REG_T_BITMASK | IMM_T_BITMASK);
            ++(*outbuf_idx);

            *(StackElem_t*) &output_code_buf[*outbuf_idx] = add_arg_num;
            *outbuf_idx += sizeof(StackElem_t);

            output_code_buf[*outbuf_idx] = register_number;
            ++(*outbuf_idx);
        }
    }

    else if (CheckMarkNeces(cmd_type))
    {
        if (asm_type == ASM_WITH_MARKS)
        {
            output_code_buf[*outbuf_idx] = (unsigned char) cmd_type;
            ++(*outbuf_idx);

            int cmd_num = 0;
            if ((cmd_num = GetMarkCmdNum(marks_arr, arg)) == -1)
            {
                printf(RED "ERROR: Meet undefined mark during assembling: %s" WHT "\n", arg);
                return UNKNOWN_MARK_NAME_ERR;
            }

            *(int*) &output_code_buf[*outbuf_idx] = cmd_num;
            *outbuf_idx += sizeof(int);
        }

        else
            *outbuf_idx += sizeof(int) + sizeof(char);
    }

    return NO_ERROR;
}


static int GetMarkCmdNum(Mark* marks_arr, const char* mark_name)
{
    for (int i = 0; i < MAX_MARKS_ARR_SIZE; ++i)
        if (strcmp(marks_arr[i].name, mark_name) == 0)
            return marks_arr[i].cmd_num;

    return -1;
}


static char GetRegisterNum(const char* reg_name)
{
    for (int i = 0; i < NUM_OF_ACCESS_REGS; ++i)
        if (strcmp(reg_name, REGISTERS[i]) == 0)
            return (char) (i+1);

    return 0;
}
