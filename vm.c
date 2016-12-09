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
#define LITERAL_MAX 32767
#define REG_MAX 32775


//static const int reg_count = NUM_REG;

static int r[NUM_REG];

static uint16_t *stack;
static uint16_t stack_size = 2 * STACK_CHUNK; 

static uint16_t *spr; // Read Stack Pointer

static uint16_t *mem;

static bool run = 1;


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

#ifdef DEBUG
    printf("VM DEBUG: Growing Stack to %d\n",stack_size);
#endif

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
    if (spr == NULL || spr-stack > stack_size) {
        spr = stack;
    }

    uint16_t val = *spr++;

    // val is register
    if (val > LITERAL_MAX && val <= REG_MAX) {
//        printf("register found: %d => %d\n",val, val-LITERAL_MAX-1);
        val = r[val-LITERAL_MAX-1];
    }
    if (val > REG_MAX) {
        val = fetch();
    }

    return val;

}

void inst_out()
{
    printf("%c", fetch());
}

void inst_jmp()
{
    uint16_t dest = fetch();
    spr = stack + dest;
}

// jt: 7 a b
// if <a> is nonzero, jump to <b>
void inst_jt()
{
	uint16_t a = fetch();
   // printf("jmp if %d non-zero\n",a);
	if (a != 0) {
		inst_jmp();
	} else {
        // burn arg
        fetch();
	}
}

// jf: 8 a b
//  if <a> is zero, jump to <b>
void inst_jf()
{
    uint16_t a = fetch();
    if (a == 0) {
        inst_jmp();
    } else {
        // burn arg
        fetch();
    }
}

// set: 1 a b
// set register <a> to the value of <b>
void inst_set() {
    uint16_t reg = fetch();
    printf("reg: %d\n",reg);
}

void exec (instruction i)
{
 //   printf("\n%d\n",i);
    switch(i) {

        case halt:
            run = 0;
            print_regs();
            break;
        case set:
            inst_set();
            break;

        case jmp:
            inst_jmp();
            break;

        case jt:
            inst_jt();
            break;

        case jf:
            inst_jf();
        break; 
            
        case out:
            inst_out();
            break;
        
        case noop:
            break;

        default:
            printf("%d\n",i);
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

void vm_read(const char *file) 
{
    FILE *bin = fopen(file, "r");
    uint16_t r;
    while (fread(&r, sizeof r, 1, bin) == 1) {
        vm_push(r);
    }
    fclose(bin);
}


int main(int argc, const char **argv)
{
    printf("Initializing Synacor VM\n");
    mem = calloc(LITERAL_MAX, sizeof *mem);
    stack = calloc(stack_size, sizeof *stack);

    printf("Mem base: %p\n", (void *)mem);
    printf("Stack   : %p\n", (void *)stack);

    const char *exec_str = NULL;
    if (argc > 1) {
        exec_str = argv[1];
        vm_read(exec_str);
    } else { 
        setup_mem();
    }

    printf("RUNNING %s\n\n\n",exec_str);
    run_vm();

    free(mem);
    free(stack);

    return 0;
}

