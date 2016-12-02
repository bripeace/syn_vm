#include <stdlib.h>
#include <stdio.h>
#include "inst.h"

#define NUM_REG 8
#define STACK_SIZE 1024
//static int stack_size = STACK_SIZE;
//static const int reg_count = NUM_REG;
static int r[NUM_REG];
static int *stack;
static int *mem;
static int run = 1;

#define MAX_INT 32767
#define WORD 2


void print_regs()
{
    for (int i = 0; i < NUM_REG; ++i)
    {
        printf("r%1d: %d\n",i, r[i]);
    }
}

void vm_push(int i)
{

}

void setup_mem()
{
    int i = 0;
    for(i = 0; i < 100; ++i)
    {
        stack[i] = noop;
    }
    stack[i] = out;
    stack[i+1] = 'b';
    stack[i+2] = out;
    stack[i+3] = 'r';
    stack[i+4] = out;
    stack[i+5] = 'a';
    stack[i+6] = out;
    stack[i+7] = 'v';
    stack[i+8] = out;
    stack[i+9] = 'e';
    stack[i+10] = out;
    stack[i+11] = 's';
    stack[i+12] = out;
    stack[i+13] = '\n';
}

int fetch()
{
    static int *sp = NULL;

    if (sp == NULL)
    {
        sp = stack;
    }

    int ret = *sp;
    ++sp;
    return ret;
}

void exec (instruction i)
{
    switch(i)
    {
        case halt:
            run = 0;
        break;
        case noop:
         //   printf("noop\n");
        break;
        case out:
            printf("%c",fetch());
        break;
        default:
        break;
    }

}

void run_vm()
{
    while (run)
    {
        instruction inst = fetch();
        exec(inst);
    }

}
int main(int argc, const char **argv)
{

    mem = calloc(MAX_INT + 1, WORD);
    stack = calloc(1024, WORD);
    
    print_regs();
    
    printf("Mem base: %p\n", (void *)mem);
    printf("Stack   : %p\n", (void *)stack);

    setup_mem();
    run_vm();
}

