#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf env;

void f1()
{
    setjmp(env);
}

int main()
{
    f1();

    longjmp(env, 1);
    
    return 0;
}
