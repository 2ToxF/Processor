// Here zoo begins

#define HANDLE_CMD_ARG_ HandleCmdArg(&my_spu);

#define DO_OPERATION_(operation) temp_num2 operation temp_num1
#define TWO_NUM_COMPARE_(sign) temp_num2 sign temp_num1
#define TWO_NUM_EQUAL_ IsEqual(temp_num2, temp_num1)

#define PUSH_NUM_(num) StackPush(my_spu.stack_num, num);
#define POP_ONE_NUM_ StackPop(my_spu.stack_num, &temp_num1);
#define POP_TWO_NUMS_ POP_ONE_NUM_ StackPop(my_spu.stack_num, &temp_num2);

#define PUSH_AFTER_OPERATION_(operation) PUSH_NUM_(DO_OPERATION_(operation))

#define JUMP_TO_MARK_ my_spu.ip = *(int*) &my_spu.code_buf[my_spu.ip];
#define CHECK_MARK_(expression)  \
    if (expression)              \
    {                            \
        JUMP_TO_MARK_            \
        break;                   \
    }                            \
                                 \
    my_spu.ip += sizeof(int);

// ------------------------------------------------------------------------------------------------------------

DEF_CMD_(HLT, 0,
{
    return NO_ERROR;
})


DEF_CMD_(PUSH, 1,
{
    HANDLE_CMD_ARG_
})

DEF_CMD_(POP, 1,
{
    HANDLE_CMD_ARG_
})


DEF_CMD_(ADD, 0,
{
    POP_TWO_NUMS_
    PUSH_AFTER_OPERATION_(+)
})

DEF_CMD_(SUB, 0,
{
    POP_TWO_NUMS_
    PUSH_AFTER_OPERATION_(-)
})

DEF_CMD_(DIV, 0,
{
    POP_TWO_NUMS_
    PUSH_AFTER_OPERATION_(/)
})

DEF_CMD_(MUL, 0,
{
    POP_TWO_NUMS_
    PUSH_AFTER_OPERATION_(*)
})

DEF_CMD_(SQRT, 0,
{
    POP_ONE_NUM_
    PUSH_NUM_(sqrt(temp_num1));
})


DEF_CMD_(IN, 0,
{
    scanf("%lf", &temp_num1);
    PUSH_NUM_(temp_num1);
})

DEF_CMD_(OUT, 0,
{
    POP_ONE_NUM_
    printf(YEL "%g" WHT "\n", temp_num1);
})

DEF_CMD_(DUMP, 0,
{
    StackDump(my_spu.stack_num, __FILE__, __LINE__);
})


DEF_CMD_(RET, 0,
{
    StackElem_t temp_ip = 0;
    StackPop(my_spu.stack_func_ret, &temp_ip);
    my_spu.ip = (int) temp_ip;
})

DEF_CMD_(CALL, 1,
{
    StackPush(my_spu.stack_func_ret, (StackElem_t) (my_spu.ip + sizeof(int)));
    JUMP_TO_MARK_
})


DEF_CMD_(JMP, 1,
{
    JUMP_TO_MARK_
})

DEF_CMD_(JE, 1,
{
    POP_TWO_NUMS_
    CHECK_MARK_(TWO_NUM_EQUAL_)
})

DEF_CMD_(JNE, 1,
{
    POP_TWO_NUMS_
    CHECK_MARK_(!TWO_NUM_EQUAL_)
})

DEF_CMD_(JA, 1,
{
    POP_TWO_NUMS_
    CHECK_MARK_(TWO_NUM_COMPARE_(>))
})

DEF_CMD_(JAE, 1,
{
    POP_TWO_NUMS_
    CHECK_MARK_(TWO_NUM_COMPARE_(>=))
})

DEF_CMD_(JB, 1,
{
    POP_TWO_NUMS_
    CHECK_MARK_(TWO_NUM_COMPARE_(<))
})

DEF_CMD_(JBE, 1,
{
    POP_TWO_NUMS_
    CHECK_MARK_(TWO_NUM_COMPARE_(<=))
})

// ------------------------------------------------------------------------------------------------------------

#undef HANDLE_CMD_ARG_

#undef DO_OPERATION_
#undef TWO_NUM_COMPARE_
#undef TWO_NUM_EQUAL_

#undef PUSH_NUM_
#undef POP_ONE_NUM_
#undef POP_TWO_NUMS_

#undef PUSH_AFTER_OPERATION_

#undef JUMP_TO_MARK_
#undef CHECK_MARK_
