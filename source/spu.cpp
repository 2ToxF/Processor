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

static const int   START_OUT_BUFSIZE = 100;
static const int   REALLOC_COEF      = 2;
static const int   REALLOC_LIMIT     = 50;

static const int   MAX_WORD_LEN = 25;
static const int   MAX_NUM_LEN  = 33;

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
static const char* JA_CMD   = "ja";
static const char* JAE_CMD  = "jae";
static const char* JB_CMD   = "jb";
static const char* JBE_CMD  = "jbe";
static const char* JE_CMD   = "je";
static const char* JNE_CMD  = "jne";

static const int   MAX_LABELS_MAS_SIZE = 10;

enum SPUComands
{
    HLT = 1,

    PUSH,
    POP,

    ADD,
    SUB,
    DIV,
    MULT,

    IN,
    OUT,

    JMP,
    JA, JAE,
    JB, JBE,
    JE, JNE,
};

struct Label
{
    char name[MAX_WORD_LEN];
    int cmd_num;
};

// ------------------------------------------------------------------------------------------------------------

static CodeError CodeAssemble(const char* input_file_name, const char* output_file_name,
                              char* asm_code_buf, Label labels_mas[MAX_LABELS_MAS_SIZE]);
static CodeError FindLabels(char* input_code_buf, int input_code_bufsize,
                            Label labels_mas[MAX_LABELS_MAS_SIZE]);
static CodeError RunCode(char* asm_code_buf, Label labels_mas[MAX_LABELS_MAS_SIZE]);

static CodeError CodeAssemble(const char* input_file_name, const char* output_file_name,
                              char** asm_code_buf, Label labels_mas[MAX_LABELS_MAS_SIZE])
{
    CodeError code_err = NO_ERROR;

    char* input_code_buf     = NULL;
    int   input_code_bufsize = 0;
    if ((code_err = MyFread(&input_code_buf, &input_code_bufsize, input_file_name)) != NO_ERROR)
        return code_err;

    FILE* out_fptr = fopen(output_file_name, "wb");
    if (out_fptr == NULL)
        return FILE_NOT_OPENED_ERR;

    if ((code_err = FindLabels(labels_mas)) != NO_ERROR)
        return code_err;

    char*  start_inp_buf = input_code_buf;
    char*  end_inp_buf   = input_code_buf + input_code_bufsize;

    size_t outbuf_size   = START_OUT_BUFSIZE;
    char*  output_buf    = (char*) calloc(outbuf_size, sizeof(char));
    size_t outbuf_idx    = 0;

    char number_str[MAX_NUM_LEN] = {};
    char word[MAX_WORD_LEN]      = {};

    while (input_code_buf < end_inp_buf)
    {
        if (sscanf(input_code_buf, "%s", word) != 0)
        {
            if (*word != ';')
            {
                if (outbuf_idx > outbuf_size - REALLOC_LIMIT)
                {
                    outbuf_size *= REALLOC_COEF;
                    output_buf = (char*) realloc(output_buf, outbuf_size);
                    memset(&output_buf[outbuf_idx], 0, outbuf_size - outbuf_idx);
                }

                if (strcmp(word, PUSH_CMD) == 0)
                {
                    input_code_buf += strlen(word);
                    sscanf(input_code_buf, "%s", number_str);
                    input_code_buf += strlen(number_str);

                    sprintf(&output_buf[outbuf_idx], "%d\n%s\n", (int) PUSH, number_str);

                    outbuf_idx += DigitsNumber((int) PUSH) + strlen(number_str) + 2;
                }

                else if (strcmp(word, ADD_CMD) == 0)
                {
                    sprintf(&output_buf[outbuf_idx], "%d\n", (int) ADD);
                    input_code_buf += strlen(word);
                    outbuf_idx += DigitsNumber((int) ADD) + 1;
                }

                else if (strcmp(word, SUB_CMD) == 0)
                {
                    sprintf(&output_buf[outbuf_idx], "%d\n", (int) SUB);
                    input_code_buf += strlen(word);
                    outbuf_idx += DigitsNumber((int) SUB) + 1;
                }

                else if (strcmp(word, DIV_CMD) == 0)
                {
                    sprintf(&output_buf[outbuf_idx], "%d\n", (int) DIV);
                    input_code_buf += strlen(word);
                    outbuf_idx += DigitsNumber((int) DIV) + 1;
                }

                else if (strcmp(word, MULT_CMD) == 0)
                {
                    sprintf(&output_buf[outbuf_idx], "%d\n", (int) MULT);
                    input_code_buf += strlen(word);
                    outbuf_idx += DigitsNumber((int) MULT) + 1;
                }

                else if (strcmp(word, IN_CMD) == 0)
                {
                    sprintf(&output_buf[outbuf_idx], "%d\n", (int) IN);
                    input_code_buf += strlen(word);
                    outbuf_idx += DigitsNumber((int) IN) + 1;
                }

                else if (strcmp(word, OUT_CMD) == 0)
                {
                    sprintf(&output_buf[outbuf_idx], "%d\n", (int) OUT);
                    input_code_buf += strlen(word);
                    outbuf_idx += DigitsNumber((int) OUT) + 1;
                }

                else if (strcmp(word, HLT_CMD) == 0)
                {
                    sprintf(&output_buf[outbuf_idx], "%d\n", (int) HLT);
                    input_code_buf += strlen(word);
                    outbuf_idx += DigitsNumber((int) HLT) + 1;
                }

                else
                {
                    return UNKNOWN_ASM_CMD_ERR;
                }

                *word = ';';
            }

            BufNextString(&input_code_buf);
        }
    }

    output_buf = char* realloc(output_buf, outbuf_idx);
    fwrite(output_buf, 1, outbuf_idx, out_fptr);
    *asm_code_buf = output_buf;

    free(start_inp_buf); start_inp_buf = NULL;

    fclose(out_fptr); out_fptr = NULL;

    return code_err;
}


static CodeError FindLabels(char* input_code_buf, int input_code_bufsize,
                            Label labels_mas[MAX_LABELS_MAS_SIZE])
{
    char* end_inp_buf        = input_code_buf + input_code_bufsize;
    char  word[MAX_WORD_LEN] = {};

    int   labels_idx         = 0;
    int   cmd_number         = 0;

    while (input_code_buf < end_inp_buf)
    {
        if (sscanf(input_code_buf, "%s", word) != 0)
        {
            if (*word != ';')
            {
                if (word[strlen(word) - 1] == ':')
                {
                    if (labels_idx < MAX_LABELS_MAS_SIZE)
                    {
                        labels_mas[labels_idx] = {0, cmd_number};
                        strcpy(labels_mas[labels_idx].name, word);
                        ++labels_idx;
                    }

                    else
                        return TOO_MUCH_LABELS_ERR;
                }

                else
                    ++cmd_number;
            }

            BufNextString(&input_code_buf);
        }
    }

    return NO_ERROR;
}


static CodeError RunCode(char* asm_code_buf, Label labels_mas[MAX_LABELS_MAS_SIZE])
{
    CodeError code_err = NO_ERROR;

    char* code_buf = NULL;
    int code_bufsize = 0;
    if ((code_err = MyFread(&code_buf, &code_bufsize, asm_code_file)) != NO_ERROR)
        return code_err;

    size_t stack = 0;
    if (CREATE_STACK(&stack) != STK_NO_ERROR)
        return STACK_ERR;

    SPUComands cmd_number = HLT;
    int number1 = 0;
    int number2 = 0;
    char* end_of_buf = code_buf + code_bufsize;

    while (code_buf < end_of_buf)
    {
        sscanf(code_buf, "%d", (int*) &cmd_number);
        switch (cmd_number)
        {
            case PUSH:
            {
                BufNextString(&code_buf);
                sscanf(code_buf, "%d", &number1);
                StackPush(stack, number1);

                break;
            }

            case ADD:
            {
                StackPop(stack, &number1);
                StackPop(stack, &number2);
                StackPush(stack, number2 + number1);

                break;
            }

            case SUB:
            {
                StackPop(stack, &number1);
                StackPop(stack, &number2);
                StackPush(stack, number2 - number1);

                break;
            }

            case DIV:
            {
                StackPop(stack, &number1);
                StackPop(stack, &number2);
                StackPush(stack, number2 / number1);

                break;
            }

            case MULT:
            {
                StackPop(stack, &number1);
                StackPop(stack, &number2);
                StackPush(stack, number2 * number1);

                break;
            }

            case IN:
            {
                scanf("%d", &number1);
                StackPush(stack, number1);

                break;
            }

            case OUT:
            {
                StackPop(stack, &number1);
                printf(YEL "%d" WHT "\n", number1);

                break;
            }

            case HLT:
                return NO_ERROR;

            default:
                return UNKNOWN_RUNTIME_CMD_ERR;
        }

        BufNextString(&code_buf);
    }

    StackDtor(&stack);

    return NO_ERROR;
}


CodeError RunMainProgram()
{
    char* asm_code_buf = NULL;
    Label labels_mas[MAX_LABELS_MAS_SIZE] = {};

    CodeError code_err = CodeAssemble(INPUT_CODE_FILE_NAME, OUTPUT_CODE_FILE_NAME, asm_code_buf, labels_mas);
    if (code_err != NO_ERROR)
        return code_err;

    int user_answer = 0;
    printf(MAG "Program assembled succesfully\nWanna run it? (Y / N)" WHT "\n");

    if (((user_answer = getchar()) == 'Y' || user_answer == 'y' || user_answer == 'N'
          || user_answer == 'n') & getchar() == '\n')
    {
        if (user_answer == 'Y' || user_answer == 'y')
        {
            printf(MAG "Starting program" WHT "\n");
            RunCode(asm_code_buf, labels_mas);
        }

        else
            printf(MAG "Stopping program" WHT "\n");
    }

    else
        printf(RED "Sorry, I don't know such command" WHT "\n");

    return code_err;
}
