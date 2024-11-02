#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#include <stdint.h>

const uint8_t CMD_BITMASK        = 0b00011111;
const uint8_t MEM_T_BITMASK      = 0b10000000;
const uint8_t REG_T_BITMASK      = 0b01000000;
const uint8_t IMM_T_BITMASK      = 0b00100000;

const int     NUM_OF_ACCESS_REGS = 4;
const int     REAL_NUM_OF_REGS   = NUM_OF_ACCESS_REGS + 1;

#define DEF_CMD_(cmd_name, cmd_num, ...) \
    CMD_##cmd_name = cmd_num,

enum SPUCommands
{
    #include "commands.h"
};

#undef DEF_CMD_

#endif
