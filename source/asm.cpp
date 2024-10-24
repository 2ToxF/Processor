#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "commands.h"
#include "input_output.h"
#include "utils.h"

// ------------------------------------------------------------------------------------------------------------

static const int     START_OUT_BUFSIZE        = 100;
static const int     REALLOC_COEF             = 2;
static const int     REALLOC_LIMIT            = 50;

static const int     MAX_CMD_LEN = 40;

static const char*   HLT_CMD_TEXT  = "hlt";
static const char*   PUSH_CMD_TEXT = "push";
static const char*   POP_CMD_TEXT  = "pop";
static const char*   ADD_CMD_TEXT  = "add";
static const char*   SUB_CMD_TEXT  = "sub";
static const char*   DIV_CMD_TEXT  = "div";
static const char*   MULT_CMD_TEXT = "mult";
static const char*   IN_CMD_TEXT   = "in";
static const char*   OUT_CMD_TEXT  = "out";
static const char*   RET_CMD_TEXT  = "ret";
static const char*   CALL_CMD_TEXT = "call";
static const char*   JMP_CMD_TEXT  = "jmp";
static const char*   JE_CMD_TEXT   = "je";
static const char*   JNE_CMD_TEXT  = "jne";
static const char*   JA_CMD_TEXT   = "ja";
static const char*   JAE_CMD_TEXT  = "jae";
static const char*   JB_CMD_TEXT   = "jb";
static const char*   JBE_CMD_TEXT  = "jbe";

static const char*   REGISTERS[NUMBER_OF_REGISTERS] = {"AX", "BX", "CX", "DX"};

static const int     MAX_MARKS_ARR_SIZE = 10;

struct Mark
{
    char name[MAX_CMD_LEN];
    int cmd_num;
};

// ------------------------------------------------------------------------------------------------------------

static bool      CheckMarkNeces(const char* cmd);
static int       PutArg        (char** input_code_buf, char** output_code_buf,
                                size_t* outbuf_idx, SPUCommands cmd_type);
static int       GetMarkCmdNum (Mark* marks_arr, const char* mark_name);
static char      GetRegisterNum(const char* reg_name);
static CodeError ScanMarks     (char* input_code_buf, int input_code_bufsize,
                                Mark marks_arr[MAX_MARKS_ARR_SIZE]);

// ------------------------------------------------------------------------------------------------------------

static bool CheckMarkNeces(const char* cmd)
{
    return strcmp(JMP_CMD_TEXT, cmd) == 0 || strcmp(JE_CMD_TEXT,   cmd) == 0 ||
           strcmp(JNE_CMD_TEXT, cmd) == 0 || strcmp(JA_CMD_TEXT,   cmd) == 0 ||
           strcmp(JAE_CMD_TEXT, cmd) == 0 || strcmp(JB_CMD_TEXT,   cmd) == 0 ||
           strcmp(JBE_CMD_TEXT, cmd) == 0 || strcmp(CALL_CMD_TEXT, cmd) == 0;
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
    if ((code_err = ScanMarks(input_code_buf, input_code_bufsize, marks_arr)) != NO_ERROR)
        return code_err;

    char*  start_inp_buf    = input_code_buf;
    char*  end_inp_buf      = input_code_buf + input_code_bufsize;

    size_t outbuf_size      = START_OUT_BUFSIZE;
    char*  output_code_buf  = (char*) calloc(outbuf_size, sizeof(char));
    size_t outbuf_idx       = 0;

    char cmd[MAX_CMD_LEN] = {};

    while (input_code_buf < end_inp_buf && sscanf(input_code_buf, "%s", cmd) != 0) // TODO: move to function
    {
        if (cmd[0] != ';') // TODO: у тебя огромное тело цикла, каждый блок ифа в отдельную функцию, какие нахуй 200 строк на функции ты ебнулся
        {
            if (cmd[strlen(cmd) - 1] == ':')
            {
                BufNextString(&input_code_buf);
                continue;
            }

            if (outbuf_idx > outbuf_size - REALLOC_LIMIT)
            {
                outbuf_size *= REALLOC_COEF;
                output_code_buf = (char*) MyRecalloc(output_code_buf, outbuf_size, outbuf_idx);
            }

            if (strcmp(cmd, HLT_CMD_TEXT) == 0)
            {
                output_code_buf[outbuf_idx] = (char) CMD_HLT;
                ++outbuf_idx;
            }

            else if (strcmp(cmd, PUSH_CMD_TEXT) == 0)
            {

            }

            else if (strcmp(cmd, POP_CMD_TEXT) == 0)
            {
                output_code_buf[outbuf_idx] = (char) CMD_POP;
                ++outbuf_idx;

                BufNextWord(&input_code_buf);
                sscanf(input_code_buf, "%s", cmd);

                char register_number = GetRegisterNum(cmd);
                if (register_number == 0)
                {
                    printf(RED "ERROR: Meet undefined register during assembling: %s" WHT "\n", cmd);
                    return UNKNOWN_REG_NAME_ERR;
                }

                output_code_buf[outbuf_idx] = register_number;
                ++outbuf_idx;
            }

            else if (strcmp(cmd, ADD_CMD_TEXT) == 0)  // TODO: delete copypaste
            {
                output_code_buf[outbuf_idx] = (char) CMD_ADD;
                ++outbuf_idx;
            }

            else if (strcmp(cmd, SUB_CMD_TEXT) == 0)
            {
                output_code_buf[outbuf_idx] = (char) CMD_SUB;
                ++outbuf_idx;
            }

            else if (strcmp(cmd, DIV_CMD_TEXT) == 0)
            {
                output_code_buf[outbuf_idx] = (char) CMD_DIV;
                ++outbuf_idx;
            }

            else if (strcmp(cmd, MULT_CMD_TEXT) == 0)
            {
                output_code_buf[outbuf_idx] = (char) CMD_MULT;
                ++outbuf_idx;
            }

            else if (strcmp(cmd, IN_CMD_TEXT) == 0)
            {
                output_code_buf[outbuf_idx] = (char) CMD_IN;
                ++outbuf_idx;
            }

            else if (strcmp(cmd, OUT_CMD_TEXT) == 0)
            {
                output_code_buf[outbuf_idx] = (char) CMD_OUT;
                ++outbuf_idx;
            }

            else if (strcmp(cmd, RET_CMD_TEXT) == 0)
            {
                output_code_buf[outbuf_idx] = (char) CMD_RET;
                ++outbuf_idx;
            }

            else if (CheckMarkNeces(cmd))
            {
                BufNextWord(&input_code_buf);
                char mark_name[MAX_CMD_LEN] = {};
                sscanf(input_code_buf, "%s", mark_name);

                int cmd_num = GetMarkCmdNum(marks_arr, mark_name);
                if (cmd_num == -1)
                {
                    printf(RED "ERROR: Meet undefined mark during assembling: %s" WHT "\n", mark_name);
                    return UNKNOWN_MARK_NAME_ERR;
                }

                else if (strcmp(JMP_CMD_TEXT, cmd) == 0)
                    output_code_buf[outbuf_idx] = (char) CMD_JMP;

                else if (strcmp(JE_CMD_TEXT, cmd) == 0)
                    output_code_buf[outbuf_idx] = (char) CMD_JE;

                else if (strcmp(JNE_CMD_TEXT, cmd) == 0)
                    output_code_buf[outbuf_idx] = (char) CMD_JNE;

                else if (strcmp(JA_CMD_TEXT, cmd) == 0)
                    output_code_buf[outbuf_idx] = (char) CMD_JA;

                else if (strcmp(JAE_CMD_TEXT, cmd) == 0)
                    output_code_buf[outbuf_idx] = (char) CMD_JAE;

                else if (strcmp(JB_CMD_TEXT, cmd) == 0)
                    output_code_buf[outbuf_idx] = (char) CMD_JB;

                else if (strcmp(JBE_CMD_TEXT, cmd) == 0)
                    output_code_buf[outbuf_idx] = (char) CMD_JBE;

                else if (strcmp(cmd, CALL_CMD_TEXT) == 0)
                    output_code_buf[outbuf_idx] = (char) CMD_CALL;

                else
                {
                    printf(RED "ERROR: Meet unknown command during assembling: %s" WHT "\n", cmd);
                    return UNKNOWN_ASM_CMD_ERR;
                }

                ++outbuf_idx;
                *((int*) (output_code_buf + outbuf_idx)) = cmd_num;
                outbuf_idx += sizeof(int);
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

    fwrite(output_code_buf, 1, outbuf_idx, out_fptr);

    free(start_inp_buf); start_inp_buf = NULL;
    fclose(out_fptr); out_fptr = NULL;

    return code_err;
}


static int PutArg(char** input_code_buf, char** output_code_buf,
                  size_t* outbuf_idx, SPUCommands cmd_type)
{
    char arg[MAX_CMD_LEN] = {};
    char add_arg_str[MAX_CMD_LEN] = {};
    char add_arg_int = 0;
    char register_number  = 0;

    BufNextWord(input_code_buf);
    sscanf(*input_code_buf, "%s", arg);

    if (StrIsNum(arg) && cmd_type == CMD_PUSH)
    {
        (*output_code_buf)[*outbuf_idx] = (char) CMD_PUSH | IMM_T_BITMASK;
        ++(*outbuf_idx);

        *(StackElem_t*) (*output_code_buf + *outbuf_idx) = (StackElem_t) atof(arg);
        *outbuf_idx += sizeof(StackElem_t);
    }

    else if ((register_number = GetRegisterNum(arg)) != 0 && (cmd_type == CMD_POP || cmd_type == CMD_PUSH))
    {
        (*output_code_buf)[*outbuf_idx] = (char) CMD_PUSH | REG_T_BITMASK;
        ++(*outbuf_idx);

        *(output_code_buf)[*outbuf_idx] = register_number;
        ++(*outbuf_idx);
    }

    else if (arg[0] == '[' && arg[strlen(arg)-1] == ']' && (cmd_type == CMD_POP || cmd_type == CMD_PUSH))
    {
        sscanf(arg, "[%[^]]]", &add_arg_str);

        if (StrIsNum(add_arg_str))
        {
            (*output_code_buf)[*outbuf_idx] = (char) CMD_PUSH | MEM_T_BITMASK | IMM_T_BITMASK;
            ++(*outbuf_idx);

            *(int*) (*output_code_buf + *outbuf_idx) = atoi(add_arg_str);
            *outbuf_idx += sizeof(int);
        }

        else if (sscanf(arg, "[%[^]]]", &add_arg_str) == 1 &&
                 (register_number = GetRegisterNum(add_arg_str)) != 0)
        {
            (*output_code_buf)[*outbuf_idx] = (char) CMD_PUSH | MEM_T_BITMASK | REG_T_BITMASK;
            ++(*outbuf_idx);

            *(output_code_buf)[*outbuf_idx] = register_number;
            ++(*outbuf_idx);
        }

        else if (sscanf(arg, "[%d+%s]", &add_arg_int, add_arg_str) == 2 ||
                 sscanf(arg, "[%[^+]+%d]", add_arg_str, &add_arg_int) == 2)
        {
            (*output_code_buf)[*outbuf_idx] = (char) CMD_PUSH | MEM_T_BITMASK | REG_T_BITMASK | IMM_T_BITMASK;
            ++(*outbuf_idx);

            *(output_code_buf)[*outbuf_idx] = register_number;
            ++(*outbuf_idx);
        }
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
    for (int i = 0; i < NUMBER_OF_REGISTERS; ++i)
        if (strcmp(reg_name, REGISTERS[i]) == 0)
            return (char) (i+1);

    return 0;
}


static CodeError ScanMarks(char* input_code_buf, int input_code_bufsize,
                            Mark marks_arr[MAX_MARKS_ARR_SIZE])
{
    char* end_inp_buf = input_code_buf + input_code_bufsize;
    char  cmd[MAX_CMD_LEN] = {};

    int   marks_idx          = 0;
    int   cmd_number         = 0;

    while (input_code_buf < end_inp_buf)
    {
        if (sscanf(input_code_buf, "%s", cmd) != 0)
        {
            if (*cmd != ';')
            {
                size_t cmd_len = strlen(cmd);

                if (cmd[cmd_len - 1] == ':')
                {
                    cmd[cmd_len - 1] = '\0';

                    if (marks_idx < MAX_MARKS_ARR_SIZE)
                    {
                        strcpy(marks_arr[marks_idx].name, cmd);
                        marks_arr[marks_idx].cmd_num = cmd_number;
                        ++marks_idx;
                    }

                    else
                        return TOO_MUCH_MARKS_ERR;
                }

                else if (StrIsNum(cmd))
                    cmd_number += sizeof(StackElem_t);

                else if (CheckMarkNeces(cmd))
                {
                    cmd_number += sizeof(int) + 1;
                    BufNextWord(&input_code_buf);
                }

                else
                    ++cmd_number;

                BufNextWord(&input_code_buf);
            }

            else
                BufNextString(&input_code_buf);
        }
    }

    // for (int i = 0; i < MAX_MARKS_ARR_SIZE; ++i)
    //     printf("Mark: %s, %d\n", marks_arr[i].name, marks_arr[i].cmd_num);

    return NO_ERROR;
}
