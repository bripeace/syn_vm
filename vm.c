#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

#include "inst.h"
#include "test.h"

#define NUM_REG 8
#define STACK_CHUNK 256

//static const int reg_count = NUM_REG;

static int r[NUM_REG];

static uint16_t *stack;
static uint16_t stack_size = 2 * STACK_CHUNK; 

static uint16_t *mem;

static bool run = 1;

#define MAX_INT 32767

void print_regs()
{
    for (int i = 0; i < NUM_REG; ++i) {
        printf("r%1d: %d\n",i, r[i]);
    }
}

bool grow_stack(uint16_t **sp)
{
    // stack may move save stackpoint offset
    size_t sp_offset = *sp-stack;

    if (stack_size > UINT16_MAX -  STACK_CHUNK) {
        //overflow
        errno = ENOMEM;
        return false;
    }

    stack_size += STACK_CHUNK;

    uint16_t *new_stack = realloc(stack, stack_size * sizeof *stack);
    if (!new_stack) {
        // couldn't grow stack
        errno = ENOMEM;
        return false;
    }

    stack = new_stack;
    *sp = stack + sp_offset;
    return true;

}

void vm_push(uint16_t i)
{
    static uint16_t *sp = NULL;

    if (sp == NULL) {
        sp = stack;
    }

    if (sp-stack >= stack_size) {
        grow_stack(&sp);
    }

    *sp++ = i;
}

void setup_mem()
{

    for(int i = 0; i < 100; ++i) {
        vm_push(noop);
    }
    vm_push(out);
    vm_push('b');
    vm_push(out);
    vm_push('\n');
    for (int i = 0; i < 921; i++) {
        vm_push(noop);
    }
    vm_push(halt);
}

int fetch()
{
    static uint16_t *sp = NULL;

    if (sp == NULL || sp-stack > stack_size) {
        sp = stack;
    }

    ++sp;
    return *(sp-1);
}

void inst_out()
{
    printf("%c", fetch());
}

void exec (instruction i)
{
    switch(i) {

        case halt:
            run = 0;
            break;

        case out:
            inst_out();
            break;

        default:
        case noop:
            break;
    }

}

void run_vm()
{
    while (run) {
        instruction inst = fetch();
        exec(inst);
    }

}

int main(int argc, const char **argv)
{
    mem = calloc(MAX_INT + 1, sizeof *mem);
    stack = calloc(stack_size, sizeof *stack);

    print_regs();

    printf("Mem base: %p\n", (void *)mem);
    printf("Stack   : %p\n", (void *)stack);

    const char *exec_str = NULL;
    if (argc > 1) {
        exec_str = argv[1];
        printf ("%s\n",exec_str);
    } else { 
        setup_mem();
    }

    run_vm();

    free(mem);
    free(stack);

    return 0;
}

