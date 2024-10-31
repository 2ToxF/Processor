#include <math.h>
#include <stdio.h>

#include "global_consts.h"
#include "input_output.h"
#include "spu.h"
#include "stack.h"
#include "stack_utils.h"

static const int MAX_RAM_SIZE = 256;

struct SPUComponents
{
    char* code_buf;
    int ip;
    char cmd;
    StackElem_t reg_arr[REAL_NUM_OF_REGS];
    StackElem_t RAM[MAX_RAM_SIZE];
    size_t stack_num;
    size_t stack_func_ret;
};

static void HandleCmdArg    (SPUComponents* my_spu);
static void HandleCmdPopArg (SPUComponents* my_spu);
static void HandleCmdPushArg(SPUComponents* my_spu);


static void HandleCmdArg(SPUComponents* my_spu)
{
    if ((my_spu->cmd & CMD_BITMASK) == CMD_PUSH)
        HandleCmdPushArg(my_spu);

    else if ((my_spu->cmd & CMD_BITMASK) == CMD_POP)
        HandleCmdPopArg(my_spu);
}


static void HandleCmdPopArg(SPUComponents* my_spu)
{
    StackElem_t arg_value = 0;

    if (my_spu->cmd & MEM_T_BITMASK)
    {
        if (my_spu->cmd & IMM_T_BITMASK)
        {
            arg_value += *(StackElem_t*) &my_spu->code_buf[my_spu->ip];
            my_spu->ip += sizeof(StackElem_t);
        }

        if (my_spu->cmd & REG_T_BITMASK)
        {
            arg_value += my_spu->reg_arr[(int) my_spu->code_buf[(my_spu->ip)++]];
        }

        StackPop(my_spu->stack_num, &my_spu->RAM[(int) arg_value]);
    }

    else
    {
        StackPop(my_spu->stack_num, &my_spu->reg_arr[(int) my_spu->code_buf[(my_spu->ip)++]]);
    }
}


static void HandleCmdPushArg(SPUComponents* my_spu)
{
    StackElem_t arg_value = 0;

    if (my_spu->cmd & IMM_T_BITMASK)
    {
        arg_value += *(StackElem_t*) &my_spu->code_buf[my_spu->ip];
        my_spu->ip += sizeof(StackElem_t);
    }

    if (my_spu->cmd & REG_T_BITMASK)
    {
        arg_value += my_spu->reg_arr[(int) my_spu->code_buf[(my_spu->ip)++]];
    }

    if (my_spu->cmd & MEM_T_BITMASK)
    {
        arg_value = my_spu->RAM[(int) arg_value];
    }

    StackPush(my_spu->stack_num, arg_value);
}


#define DEF_CMD_(cmd_name, cmd_num, args_num, code)  \
    case CMD_##cmd_name:                             \
        {code}                                       \
        break;

CodeError RunCode(const char* asm_file_name)
{
    CodeError code_err = NO_ERROR;
    SPUComponents my_spu = {};

    int code_bufsize = 0;
    if ((code_err = MyFread(&my_spu.code_buf, &code_bufsize, asm_file_name)) != NO_ERROR)
        return code_err;

    if (CREATE_STACK(&my_spu.stack_num) != STK_NO_ERROR)
        return STACK_ERR;

    if (CREATE_STACK(&my_spu.stack_func_ret) != STK_NO_ERROR)
        return STACK_ERR;

    StackElem_t temp_num1 = 0;
    StackElem_t temp_num2 = 0;

    while (true)
    {
        my_spu.cmd = my_spu.code_buf[my_spu.ip++];

        switch (my_spu.cmd & CMD_BITMASK)
        {
            #include "commands.h"

            default:
            {
                printf(RED "ERROR: Meet undefined command during processing: %d.\n"
                           "Conjuction with bitmask: %d" WHT "\n", my_spu.cmd, my_spu.cmd & CMD_BITMASK);
                return UNKNOWN_RUNTIME_CMD_ERR;
            }
        }
    }

    StackDtor(&my_spu.stack_num);
    StackDtor(&my_spu.stack_func_ret);

    return NO_ERROR;
}

#undef DEF_CMD_
