#ifndef COMMANDS_H
#define COMMANDS_H

#include "settings.h"
#include "stdint.h"

const uint8_t CMD_BITMASK         = 0b00011111;
const uint8_t MEM_T_BITMASK       = 0b10000000;
const uint8_t REG_T_BITMASK       = 0b01000000;
const uint8_t IMM_T_BITMASK       = 0b00100000;

const int     NUMBER_OF_REGISTERS = 4;

enum SPUCommands
{
    CMD_HLT = -1,

    CMD_PUSH,
    CMD_POP,

    CMD_ADD,
    CMD_SUB,
    CMD_DIV,
    CMD_MULT,
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
