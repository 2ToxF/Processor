#include "input_output.h"
#include "runner.h"

int main(int argc, char* argv[])
{
    CodeError code_err = RunMainProgram(argc, argv);
    PrintCodeError(code_err);
    return code_err;
}
