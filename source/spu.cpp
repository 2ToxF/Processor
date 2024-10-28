#include <math.h>
#include <stdio.h>

#include "global_consts.h"
#include "input_output.h"
#include "spu.h"
#include "stack.h"
#include "stack_utils.h"

static const int MAX_RAM_SIZE = 256;

static void HandleCmdArg    (char* code_buf, int* ip_ptr, char cmd_type,
                             StackElem_t reg_arr[NUM_OF_ACCESS_REGS + 1], StackElem_t RAM[MAX_RAM_SIZE],
                             size_t stack_num);
static void HandleCmdPopArg (char* code_buf, int* ip_ptr, char cmd_type,
                             StackElem_t reg_arr[NUM_OF_ACCESS_REGS + 1], StackElem_t RAM[MAX_RAM_SIZE],
                             size_t stack_num);
static void HandleCmdPushArg(char* code_buf, int* ip_ptr, char cmd_type,
                             StackElem_t reg_arr[NUM_OF_ACCESS_REGS + 1], StackElem_t RAM[MAX_RAM_SIZE],
                             size_t stack_num);


static void HandleCmdArg(char* code_buf, int* ip_ptr, char cmd_type,
                         StackElem_t reg_arr[NUM_OF_ACCESS_REGS + 1], StackElem_t RAM[MAX_RAM_SIZE],
                         size_t stack_num)
{
    if ((cmd_type & CMD_BITMASK) == CMD_PUSH)
        HandleCmdPushArg(code_buf, ip_ptr, cmd_type, reg_arr, RAM, stack_num);

    else if ((cmd_type & CMD_BITMASK) == CMD_POP)
        HandleCmdPopArg(code_buf, ip_ptr, cmd_type, reg_arr, RAM, stack_num);
}


static void HandleCmdPopArg(char* code_buf, int* ip_ptr, char cmd_type,
                            StackElem_t reg_arr[NUM_OF_ACCESS_REGS + 1], StackElem_t RAM[MAX_RAM_SIZE],
                            size_t stack_num)
{
    StackElem_t arg_value = 0;

    if (cmd_type & MEM_T_BITMASK)
    {
        if (cmd_type & IMM_T_BITMASK)
        {
            arg_value += *(StackElem_t*) &code_buf[*ip_ptr];
            *ip_ptr += sizeof(StackElem_t);
        }

        if (cmd_type & REG_T_BITMASK)
        {
            arg_value += reg_arr[(int) code_buf[(*ip_ptr)++]];
        }

        StackPop(stack_num, &RAM[(int) arg_value]);
    }

    else
    {
        StackPop(stack_num, &reg_arr[(int) code_buf[(*ip_ptr)++]]);
    }
}


static void HandleCmdPushArg(char* code_buf, int* ip_ptr, char cmd_type,
                             StackElem_t reg_arr[NUM_OF_ACCESS_REGS + 1], StackElem_t RAM[MAX_RAM_SIZE],
                             size_t stack_num)
{
    StackElem_t arg_value = 0;

    if (cmd_type & IMM_T_BITMASK)
    {
        arg_value += *(StackElem_t*) &code_buf[*ip_ptr];
        *ip_ptr += sizeof(StackElem_t);
    }

    if (cmd_type & REG_T_BITMASK)
    {
        arg_value += reg_arr[(int) code_buf[(*ip_ptr)++]];
    }

    if (cmd_type & MEM_T_BITMASK)
    {
        arg_value = RAM[(int) arg_value];
    }

    StackPush(stack_num, arg_value);
}


#define DEF_CMD_(cmd_name, cmd_num, args_num, code)  \
    case CMD_##cmd_name:                             \
        {code}                                       \
        break;

CodeError RunCode(const char* asm_file_name)
{
    CodeError code_err = NO_ERROR;

    char* code_buf = NULL;
    int code_bufsize = 0;
    if ((code_err = MyFread(&code_buf, &code_bufsize, asm_file_name)) != NO_ERROR)
        return code_err;

    size_t stack_num = 0;
    if (CREATE_STACK(&stack_num) != STK_NO_ERROR)
        return STACK_ERR;

    size_t stack_func_ret = 0;
    if (CREATE_STACK(&stack_func_ret) != STK_NO_ERROR)
        return STACK_ERR;

    StackElem_t reg_arr[NUM_OF_ACCESS_REGS+1] = {};
    StackElem_t RAM[MAX_RAM_SIZE]             = {};

    int ip = 0;
    StackElem_t temp_num1 = 0;
    StackElem_t temp_num2 = 0;

    while (true)
    {
        char cmd = code_buf[ip++];

        switch (cmd & CMD_BITMASK)
        {
            #include "commands.h"

            default:
            {
                printf(RED "ERROR: Meet undefined command during processing: %d.\n"
                           "Conjuction with bitmask: %d" WHT "\n", cmd, cmd & CMD_BITMASK);
                return UNKNOWN_RUNTIME_CMD_ERR;
            }
        }
    }

    StackDtor(&stack_num);
    StackDtor(&stack_func_ret);

    return NO_ERROR;
}

#undef DEF_CMD_
