#include <stdlib.h>
#include <stdio.h>

#define NUM_REG 8
const int reg_count = NUM_REG;
static int r[NUM_REG];
static int *stack;
static int *mem;

#define MAX_INT 32767
#define WORD 2


void print_regs()
{
    for (int i = 0; i < NUM_REG; ++i)
    {
        printf("r%1d: %d\n",i, r[i]);
    }
}

int main(int argc, const char **argv)
{

    mem = calloc(MAX_INT + 1, WORD);
    stack = calloc(1024, WORD);
    
    print_regs();
    
    printf("Mem base: %p\n", (void *)mem);
    printf("Stack   : %p\n", (void *)stack);
}

