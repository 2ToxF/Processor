#ifndef COMMANDS_H
#define COMMANDS_H

#include "settings.h"
#include "stdint.h"

enum SPUCommands
{
    CMD_HLT = 0,

    CMD_PUSH,
    CMD_POP,

    CMD_ADD,
    CMD_SUB,
    CMD_DIV,
    CMD_MUL,
    CMD_SQRT,

    CMD_IN,
    CMD_OUT,

    CMD_RET,
    CMD_CALL,

    CMD_JMP,
    CMD_JE, CMD_JNE,
    CMD_JA, CMD_JAE,
    CMD_JB, CMD_JBE,
};

#endif
