#include <stdio.h>


int main()
{
    char str[10] = {};
    int number = 0;


    scanf("[%[^+]+%d]", str, &number);
    printf("\n%s   %d\n", str, number);

    return 0;
}
