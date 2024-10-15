#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include "settings.h"

#define BLK "\033[0;30m"
#define RED "\033[0;31m"
#define GRN "\033[0;32m"
#define YEL "\033[0;33m"
#define BLU "\033[0;34m"
#define MAG "\033[0;35m"
#define CYN "\033[0;36m"
#define WHT "\033[0;37m"

CodeError MyFread(char** input_buffer, int* input_buffer_length, const char* input_file_name);
void PrintCodeError(CodeError code_err);

#endif
