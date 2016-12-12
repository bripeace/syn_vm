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
static uint16_t *sp = NULL;


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

    if (sp == NULL) {
        sp = stack;
    }

    if (sp-stack >= stack_size) {
        grow_stack(&sp);
    }

    *sp++ = i;
}

uint16_t vm_pop()
{
    uint16_t i;
    if (sp == NULL) {
        i = REG_MAX+1;
    }
    i = *--sp;
    return i;
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

    return val;
}

bool is_register(uint16_t val) {
    return val > LITERAL_MAX && val <= REG_MAX;
}

// read VAL. i.e return literal or register value
uint16_t vm_read(uint16_t val) {

    // val is register
    if (is_register(val)) {
        val = r[val-LITERAL_MAX-1];
    }
/*    if (val > REG_MAX) {
        val = fetch();
    } */
    return val;
}

bool vm_write(uint16_t dest, uint16_t val) {
    if (is_register(dest)) {
        r[dest-LITERAL_MAX-1] = val;
        return true;
    }
    return false;
}


// set: 1 a b
// set register <a> to the value of <b>
void inst_set() {
    uint16_t reg = fetch();
    uint16_t val = vm_read(fetch());
    vm_write(reg, val);
}

// push: 2 a
// push <a> onto the stack
void inst_push() {
    uint16_t a = vm_read(fetch());
    vm_push(a);
}

// pop: 3 a
// remove the top element from the stack and write it into <a>; empty stack = error
void inst_pop() {
    uint16_t a = fetch(); 
    uint16_t b = vm_pop();  
    vm_write(a, b);
}

// eq: 4 a b c
// set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
void inst_eq()
{
    uint16_t a = fetch();
    uint16_t b = vm_read(fetch());
    uint16_t c = vm_read(fetch());
    vm_write(a, b == c ? 1 : 0);
}

// gt: 5 a b c
// set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
void inst_gt()
{
   uint16_t a = fetch();
   uint16_t b = vm_read(fetch());
   uint16_t c = vm_read(fetch());
   vm_write(a, b > c ? 1 : 0);
}

// jmp: 6 a
// jump to <a>
void inst_jmp()
{
    uint16_t dest = vm_read(fetch());
    spr = stack + dest;
}

// jt: 7 a b
// if <a> is nonzero, jump to <b>
void inst_jt()
{
	uint16_t a = vm_read(fetch());
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
    uint16_t a = vm_read(fetch());
    if (a == 0) {
        inst_jmp();
    } else {
        // burn arg
        fetch();
    }
}

// add: 9 a b c
// assign into <a> the sum of <b> and <c> (modulo 32768)
void inst_add() {
    uint16_t a = fetch();
    uint16_t b = vm_read(fetch());
    uint16_t c = vm_read(fetch());
    vm_write(a, (b+c)%(LITERAL_MAX+1));
}

// mult: 10 a b c
// store into <a> the product of <b> and <c> (modulo 32768)
void inst_mult() {
    uint16_t a = fetch();
    uint16_t b = vm_read(fetch());
    uint16_t c = vm_read(fetch());
    vm_write(a, (b*c) % (LITERAL_MAX + 1));
}

// mod: 11 a b c
// store into <a> the remainder of <b> divided by <c>
void inst_mod() {
    uint16_t a = fetch();
    uint16_t b = vm_read(fetch());
    uint16_t c = vm_read(fetch());
    vm_write(a, b % c);
}

// and: 12 a b c
// stores into <a> the bitwise and of <b> and <c>
void inst_and() {
    uint16_t a = fetch();
    uint16_t b = vm_read(fetch());
    uint16_t c = vm_read(fetch());
    vm_write(a, b & c);
    
}

// or: 13 a b c
// stores into <a> the bitwise or of <b> and <c>
void inst_or() {
    uint16_t a = fetch();
    uint16_t b = vm_read(fetch());
    uint16_t c = vm_read(fetch());
    vm_write(a, b | c);
}

// not: 14 a b
// stores 15-bit bitwise inverse of <b> in <a>
void inst_not() {
    uint16_t a = fetch();
    uint16_t b = vm_read(fetch());
    uint16_t i = 0x7fff & ~b;
    vm_write(a, i);
}

// rmem: 15 a b
// read memory at address <b> and write it to <a>
void inst_rmem() {
    uint16_t a = fetch();
    uint16_t b = vm_read(fetch());
    vm_write(a, mem[b]);
}

// call: 17 a
// write the address of the next instruction to the stack and jump to <a>
void inst_call() {
    vm_push(spr-stack+1);
    inst_jmp();
}

// out: 19 a
// write the character represented by ascii code <a> to the terminal
void inst_out()
{
    printf("%c", vm_read(fetch()));
}

void exec (instruction i)
{
 //  printf("%d\n",i);
    switch(i) {

        case halt:
            run = 0;
            print_regs();
            break;

        case set:
            inst_set();
            break;

        case push:
            inst_push();
            break;

        case pop:
            inst_pop();
            break;

        case eq:
            inst_eq();
            break;
        
        case gt:
            inst_gt();
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
            
        case add:
            inst_add();
            break;

        case mult:
            inst_mult();
            break;

        case mod:
            inst_mod();
            break;

        case and:
            inst_and();
            break;

        case or:
            inst_or();
            break;

        case not:
           inst_not();
            break;

        case rmem:
            inst_rmem();
            break;

        case call:
            inst_call();
            break;

        case out:
            inst_out();
            break;

        case noop:
            break;
        
        default:
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

void vm_load(const char *file) 
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
        vm_load(exec_str);
    } else { 
        setup_mem();
    }

    printf("RUNNING %s\n\n\n",exec_str);
    run_vm();

    free(mem);
    free(stack);

    return 0;
}

