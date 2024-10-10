#ifndef SPU_H
#define SPU_H

#include "settings.h"

CodeError CodeAssemble(const char input_file_name[]);
CodeError MyFread(char** input_buffer, int* input_buffer_length, char* input_file_name)
CodeError RunProgram();

#endif
