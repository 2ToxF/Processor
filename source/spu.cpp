#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input_output.h"
#include "spu.h"
#include "stack.h"
#include "utils.h"

// ------------------------------------------------------------------------------------------------------------

static const char* INPUT_CODE_FILE_NAME  = "input_code_v1.asm";
static const char* OUTPUT_CODE_FILE_NAME = "output_code";
static const char  NULL_TARGET = '\0';

static const int   START_OUT_BUFSIZE        = 100;
static const int   REALLOC_COEF             = 2;
static const int   REALLOC_LIMIT            = 50;

static const int   MAX_INSTR_LEN = 40;

static const char* PUSH_CMD = "push";
static const char* POP_CMD  = "pop";
static const char* ADD_CMD  = "add";
static const char* SUB_CMD  = "sub";
static const char* DIV_CMD  = "div";
static const char* MULT_CMD = "mult";
static const char* IN_CMD   = "in";
static const char* OUT_CMD  = "out";
static const char* HLT_CMD  = "hlt";
static const char* JMP_CMD  = "jmp";
static const char* JE_CMD   = "je";
static const char* JNE_CMD  = "jne";
static const char* JA_CMD   = "ja";
static const char* JAE_CMD  = "jae";
static const char* JB_CMD   = "jb";
static const char* JBE_CMD  = "jbe";

static const int   NUMBER_OF_REGISTERS = 4;
static const char* REGISTERS[NUMBER_OF_REGISTERS] = {"AX", "BX", "CX", "DX"};

static const int   MAX_MARKS_MAS_SIZE = 10;

enum SPUComands
{
    HLT = 1,

    PUSH, PUSHR,
    POP,

    ADD,
    SUB,
    DIV,
    MULT,

    IN,
    OUT,

    JMP,
    JE, JNE,
    JA, JAE,
    JB, JBE,
};

struct Mark
{
    char name[MAX_INSTR_LEN];
    int cmd_num;
};

// ------------------------------------------------------------------------------------------------------------

static CodeError CodeAssemble  (const char* input_file_name, const char* output_file_name);
static int       GetMarkCmdNum (Mark* marks_mas, const char* mark_name);
static char      GetRegisterNum(const char* reg_name);
static CodeError RunCode       (const char* asm_file_name);
static CodeError ScanMarks     (char* input_code_buf, int input_code_bufsize,
                                Mark marks_mas[MAX_MARKS_MAS_SIZE]);

static CodeError CodeAssemble(const char* input_file_name, const char* output_file_name)
{
    CodeError code_err = NO_ERROR;

    char* input_code_buf     = NULL;
    int   input_code_bufsize = 0;
    if ((code_err = MyFread(&input_code_buf, &input_code_bufsize, input_file_name)) != NO_ERROR)
        return code_err;

    FILE* out_fptr = fopen(output_file_name, "wb");
    if (out_fptr == NULL)
        return FILE_NOT_OPENED_ERR;

    Mark marks_mas[MAX_MARKS_MAS_SIZE] = {};
    if ((code_err = ScanMarks(input_code_buf, input_code_bufsize, marks_mas)) != NO_ERROR)
        return code_err;

    char*  start_inp_buf    = input_code_buf;
    char*  end_inp_buf      = input_code_buf + input_code_bufsize;

    size_t outbuf_size      = START_OUT_BUFSIZE;
    char*  output_code_buf  = (char*) calloc(outbuf_size, sizeof(char));
    size_t outbuf_idx       = 0;

    char instruction[MAX_INSTR_LEN] = {};

    while (input_code_buf < end_inp_buf && sscanf(input_code_buf, "%s", instruction) != 0)
    {
        if (instruction[0] != ';')
        {
            if (instruction[strlen(instruction) - 1] == ':')
            {
                BufNextString(&input_code_buf);
                continue;
            }

            if (outbuf_idx > outbuf_size - REALLOC_LIMIT)
            {
                outbuf_size *= REALLOC_COEF;
                output_code_buf = (char*) realloc(output_code_buf, outbuf_size);
                memset(&output_code_buf[outbuf_idx], 0, outbuf_size - outbuf_idx);
            }


            if (strcmp(instruction, PUSH_CMD) == 0)
            {
                output_code_buf[outbuf_idx] = (char) PUSH;
                ++outbuf_idx;

                BufNextWord(&input_code_buf);
                sscanf(input_code_buf, "%s", instruction);

                if (StrIsNum(instruction))
                {
                    *(double*) (output_code_buf + outbuf_idx) = atof(instruction);

                    // for (int i = 0; i < 8; ++i)
                    //     printf("%d ", output_code_buf[outbuf_idx + i]);

                    // printf("\n" YEL "%lf" WHT "\n", *((double*) (output_code_buf + outbuf_idx)));
                    // printf(BLU "%llu: %s = %lf" WHT "\n\n", outbuf_idx, instruction, atof(instruction));
                    outbuf_idx += sizeof(double);
                }

                else
                {
                    char register_number = GetRegisterNum(instruction);
                    if (register_number == 0)
                        return UNKNOWN_REG_NAME_ERR;

                    output_code_buf[outbuf_idx] = register_number;
                    ++outbuf_idx;
                }
            }

            else if (strcmp(instruction, POP_CMD) == 0)
            {
                output_code_buf[outbuf_idx] = (char) PUSH;
                ++outbuf_idx;

                BufNextWord(&input_code_buf);
                sscanf(input_code_buf, "%s", instruction);

                char register_number = GetRegisterNum(instruction);
                if (register_number == 0)
                    return UNKNOWN_REG_NAME_ERR;

                output_code_buf[outbuf_idx] = register_number;
                ++outbuf_idx;
            }

            else if (strcmp(instruction, ADD_CMD) == 0)
            {
                output_code_buf[outbuf_idx] = (char) ADD;
                ++outbuf_idx;
            }

            else if (strcmp(instruction, SUB_CMD) == 0)
            {
                output_code_buf[outbuf_idx] = (char) SUB;
                ++outbuf_idx;
            }

            else if (strcmp(instruction, DIV_CMD) == 0)
            {
                output_code_buf[outbuf_idx] = (char) DIV;
                ++outbuf_idx;
            }

            else if (strcmp(instruction, MULT_CMD) == 0)
            {
                output_code_buf[outbuf_idx] = (char) MULT;
                ++outbuf_idx;
            }

            else if (strcmp(instruction, IN_CMD) == 0)
            {
                output_code_buf[outbuf_idx] = (char) IN;
                ++outbuf_idx;
            }

            else if (strcmp(instruction, OUT_CMD) == 0)
            {
                output_code_buf[outbuf_idx] = (char) OUT;
                ++outbuf_idx;
            }

            else if (strcmp(instruction, HLT_CMD) == 0)
            {
                output_code_buf[outbuf_idx] = (char) HLT;
                ++outbuf_idx;
            }

            else if (instruction[0] == 'j')
            {
                BufNextWord(&input_code_buf);
                char mark_name[MAX_INSTR_LEN] = {};
                sscanf(input_code_buf, "%s", mark_name);

                int cmd_num = GetMarkCmdNum(marks_mas, mark_name);
                if (cmd_num == -1)
                {
                    printf(RED "ERROR: Meet undefined mark during assembling: %s" WHT "\n", mark_name);
                    return UNKNOWN_MARK_NAME_ERR;
                }

                else if (strcmp(JMP_CMD, instruction) == 0)
                    output_code_buf[outbuf_idx] = (char) JMP;

                else if (strcmp(JE_CMD, instruction) == 0)
                    output_code_buf[outbuf_idx] = (char) JE;

                else if (strcmp(JNE_CMD, instruction) == 0)
                    output_code_buf[outbuf_idx] = (char) JNE;

                else if (strcmp(JA_CMD, instruction) == 0)
                    output_code_buf[outbuf_idx] = (char) JA;

                else if (strcmp(JAE_CMD, instruction) == 0)
                    output_code_buf[outbuf_idx] = (char) JAE;

                else if (strcmp(JB_CMD, instruction) == 0)
                    output_code_buf[outbuf_idx] = (char) JB;

                else if (strcmp(JBE_CMD, instruction) == 0)
                    output_code_buf[outbuf_idx] = (char) JBE;

                else
                {
                    printf(RED "ERROR: Meet unknown command during assembling: %s" WHT "\n", instruction);
                    return UNKNOWN_ASM_CMD_ERR;
                }

                ++outbuf_idx;
                *((int*) (output_code_buf + outbuf_idx)) = cmd_num;
                outbuf_idx += sizeof(int);
            }

            else
            {
                printf(RED "ERROR: Meet unknown command during assembling: %s" WHT "\n", instruction);
                return UNKNOWN_ASM_CMD_ERR;
            }
        }

        BufNextString(&input_code_buf);
    }

    fwrite(output_code_buf, 1, outbuf_idx, out_fptr);

    free(start_inp_buf); start_inp_buf = NULL;
    fclose(out_fptr); out_fptr = NULL;

    return code_err;
}


static int GetMarkCmdNum(Mark* marks_mas, const char* mark_name)
{
    for (int i = 0; i < MAX_MARKS_MAS_SIZE; ++i)
        if (strcmp(marks_mas[i].name, mark_name) == 0)
            return marks_mas[i].cmd_num;

    return -1;
}


static char GetRegisterNum(const char* reg_name)
{
    for (int i = 0; i < NUMBER_OF_REGISTERS; ++i)
        if (strcmp(reg_name, REGISTERS[i]) == 0)
            return (char) (i+1);

    return 0;
}


static CodeError RunCode(const char* asm_file_name)
{
    CodeError code_err = NO_ERROR;

    char* code_buf = NULL;
    int code_bufsize = 0;
    if ((code_err = MyFread(&code_buf, &code_bufsize, asm_file_name)) != NO_ERROR)
        return code_err;

    size_t stack = 0;
    if (CREATE_STACK(&stack) != STK_NO_ERROR)
        return STACK_ERR;

    int ip           = 0;
    double temp_num1 = 0;
    double temp_num2 = 0;

    while (true)
    {
        switch (code_buf[ip++])
        {
            case PUSH:
            {
                StackPush(stack, *((double*) (code_buf + ip)));
                ip += sizeof(double);
                break;
            }

            case ADD:
            {
                StackPop(stack, &temp_num1);
                StackPop(stack, &temp_num2);
                StackPush(stack, temp_num2 + temp_num1);

                break;
            }

            case SUB:
            {
                StackPop(stack, &temp_num1);
                StackPop(stack, &temp_num2);
                StackPush(stack, temp_num2 - temp_num1);

                break;
            }

            case DIV:
            {
                StackPop(stack, &temp_num1);
                StackPop(stack, &temp_num2);
                StackPush(stack, temp_num2 / temp_num1);

                break;
            }

            case MULT:
            {
                StackPop(stack, &temp_num1);
                StackPop(stack, &temp_num2);
                StackPush(stack, temp_num2 * temp_num1);

                break;
            }

            case IN:
            {
                scanf("%lf", &temp_num1);
                StackPush(stack, temp_num1);

                break;
            }

            case OUT:
            {
                StackPop(stack, &temp_num1);
                printf(YEL "%g" WHT "\n", temp_num1);

                break;
            }

            case HLT:
                return NO_ERROR;

            default:
                return UNKNOWN_RUNTIME_CMD_ERR;
        }
    }

    StackDtor(&stack);

    return NO_ERROR;
}


CodeError RunMainProgram()
{
    CodeError code_err = CodeAssemble(INPUT_CODE_FILE_NAME, OUTPUT_CODE_FILE_NAME);
    if (code_err != NO_ERROR)
        return code_err;

    int user_answer = 0;
    printf(MAG "Program assembled succesfully\nWanna run it? (Y / N)" WHT "\n");

    if (((user_answer = getchar()) == 'Y' || user_answer == 'y' || user_answer == 'N'
          || user_answer == 'n') & (getchar() == '\n'))
    {
        if (user_answer == 'Y' || user_answer == 'y')
        {
            printf(MAG "Starting program" WHT "\n");
            RunCode(OUTPUT_CODE_FILE_NAME);
        }

        else
            printf(MAG "Stopping program" WHT "\n");
    }

    else
        printf(RED "Sorry, I don't know such command" WHT "\n");

    return code_err;
}


static CodeError ScanMarks(char* input_code_buf, int input_code_bufsize,
                            Mark marks_mas[MAX_MARKS_MAS_SIZE])
{
    char* end_inp_buf        = input_code_buf + input_code_bufsize;
    char  instruction[MAX_INSTR_LEN] = {};

    int   marks_idx         = 0;
    int   cmd_number         = 0;

    while (input_code_buf < end_inp_buf && sscanf(input_code_buf, "%s", instruction) != 0)
    {
        if (*instruction != ';')
        {
            if (instruction[strlen(instruction) - 1] == ':')
            {
                if (marks_idx < MAX_MARKS_MAS_SIZE)
                {
                    strcpy(marks_mas[marks_idx].name, instruction);
                    marks_mas[marks_idx].cmd_num = cmd_number;
                    ++marks_idx;
                }

                else
                    return TOO_MUCH_MARKS_ERR;
            }

            else
                ++cmd_number;

            BufNextWord(&input_code_buf);
        }

        else
            BufNextString(&input_code_buf);
    }

    return NO_ERROR;
}
