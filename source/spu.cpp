#include <math.h>
#include <stdio.h>

#include "input_output.h"
#include "commands.h"
#include "spu.h"
#include "stack.h"
#include "stack_utils.h"


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
        // printf(GRN "%d: %d" WHT "\n", ip, code_buf[ip]);
        switch (code_buf[ip++])
        {
            case CMD_HLT:
                return NO_ERROR;

            case (CMD_PUSH | IMM_T_BITMASK): // TODO: проверяй только последний бит который отвечает за тип команды, дальше HandlePush и все проверки там
            {
                StackPush(stack_num, *(StackElem_t*) &code_buf[ip]);
                ip += sizeof(StackElem_t);
                break;
            }

            case (CMD_PUSH | REG_T_BITMASK):
            {
                StackPush(stack_num, reg_arr[(int) code_buf[ip]]);
                ++ip;
                break;
            }

            case CMD_POP:
            {
                StackPop(stack_num, &reg_arr[(int) code_buf[ip]]);
                ++ip;
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

            case CMD_SQRT:
            {
                StackPop(stack_num, &temp_num1);
                StackPush(stack_num, sqrt(temp_num1));

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

    return NO_ERROR;
}
