#include "input_output.h"
#include "runner.h"

int main()
{
    CodeError code_err = RunMainProgram();
    PrintCodeError(code_err);
    return code_err;
}
