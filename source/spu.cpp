#include <stdio.h>

#include "input_output.h"
#include "commands.h"
#include "spu.h"
#include "stack.h"
#include "stack_utils.h"

static StackElem_t HandleArg(const char* code_buf, int* ip, SPUCommands cmd_type,
                             StackElem_t reg_arr[NUMBER_OF_REGISTERS+1]);


static StackElem_t HandleArg(const char* code_buf, int* ip, SPUCommands cmd_type,
                             StackElem_t reg_arr[NUMBER_OF_REGISTERS+1], StackElem_t RAM[])
{
    StackElem_t arg_value = 0;

    if (cmd_type & IMM_T_BITMASK)
    {
        arg_value += *(StackElem_t) (code_buf + ip);
        ++(*ip);
    }

    if (cmd_type & REG_T_BITMASK)
    {
        arg_value += codebuf[*ip];
        ++(*ip);
    }

    if (cmd_type & MEM_T_BITMASK)
        arg_value = RAM[(int) arg_value];

    return arg_value;
}


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

    StackElem_t reg_arr[NUMBER_OF_REGISTERS+1] = {};

    int ip = 0;
    StackElem_t temp_num1 = 0;
    StackElem_t temp_num2 = 0;

    while (true) // TODO: поразбивай на функции и мб кодогенерацию
    {
        printf(GRN "%d: %d" WHT "\n", ip, code_buf[ip]);
        switch (code_buf[ip++] & IMM_T_BITMASK)
        {
            case CMD_HLT:
                return NO_ERROR;

            case (CMD_PUSH): // TODO: проверяй только последний бит который отвечает за тип команды, дальше HandlePush и все проверки там
            {
                HandleArg(code_buf, ip, CMD_PUSH);
                break;
            }

            case CMD_POP:
            {
                HandleArg(code_buf, ip, CMD_POP);
                break;
            }

            case CMD_ADD:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);
                StackPush(stack_num, temp_num2 + temp_num1);

                break;
            }

            case CMD_SUB:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);
                StackPush(stack_num, temp_num2 - temp_num1);

                break;
            }

            case CMD_DIV:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);
                StackPush(stack_num, temp_num2 / temp_num1);

                break;
            }

            case CMD_MULT:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);
                StackPush(stack_num, temp_num2 * temp_num1);

                break;
            }

            case CMD_IN:
            {
                scanf("%lf", &temp_num1);
                StackPush(stack_num, temp_num1);

                break;
            }

            case CMD_OUT:
            {
                StackPop(stack_num, &temp_num1);
                printf(YEL "%g" WHT "\n", temp_num1);

                break;
            }

            case CMD_RET:
            {
                StackElem_t temp_ip = 0;
                StackPop(stack_func_ret, &temp_ip);
                ip = (int) temp_ip;

                break;
            }

            case CMD_CALL:
            {
                StackPush(stack_func_ret, (StackElem_t) (ip + sizeof(int)));
                ip = *(int*) &code_buf[ip];
                break;
            }

            case CMD_JMP:
            {
                ip = *(int*) &code_buf[ip];
                break;
            }

            case CMD_JE:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);

                if (IsEqual(temp_num2, temp_num1))
                {
                    ip = *(int*) &code_buf[ip];
                    break;
                }

                ip += sizeof(int);
                break;
            }

            case CMD_JNE:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);

                if (!IsEqual(temp_num2, temp_num1))
                {
                    ip = *(int*) &code_buf[ip];
                    break;
                }

                ip += sizeof(int);
                break;
            }

            case CMD_JA:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);

                if (temp_num2 > temp_num1)
                {
                    ip = *(int*) &code_buf[ip];
                    break;
                }

                ip += sizeof(int);
                break;
            }

            case CMD_JAE:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);

                if (temp_num2 >= temp_num1)
                {
                    ip = *(int*) &code_buf[ip];
                    break;
                }

                ip += sizeof(int);
                break;
            }

            case CMD_JB:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);

                if (temp_num2 < temp_num1)
                {
                    ip = *(int*) &code_buf[ip];
                    break;
                }

                ip += sizeof(int);
                break;
            }

            case CMD_JBE:
            {
                StackPop(stack_num, &temp_num1);
                StackPop(stack_num, &temp_num2);

                if (temp_num2 <= temp_num1)
                {
                    ip = *(int*) &code_buf[ip];
                    break;
                }

                ip += sizeof(int);
                break;
            }

            default:
            {
                printf(RED "ERROR: Meet undefined command during processing: %d" WHT "\n", code_buf[ip-1]);
                return UNKNOWN_RUNTIME_CMD_ERR;
            }
        }
    }

    StackDtor(&stack_num);
    StackDtor(&stack_func_ret);

    return NO_ERROR;
}
