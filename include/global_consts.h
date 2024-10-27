#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#include <stdint.h>

const uint8_t CMD_BITMASK        = 0b00011111;
const uint8_t MEM_T_BITMASK      = 0b10000000;
const uint8_t REG_T_BITMASK      = 0b01000000;
const uint8_t IMM_T_BITMASK      = 0b00100000;

const int     NUM_OF_ACCESS_REGS = 4;

#endif
