#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include "inst.h"

extern bool grow_stack(uint16_t **sp);
extern void exec(instruction i);
extern int run;
extern uint16_t *stack;
extern uint16_t stack_size;

static void test_halt_inst(void **state)
{
    (void) state;
    run = 1;
    exec(halt);

    assert_int_equal(run, 0);
}

static void test_grow_stack(void **state)
{
    (void) state;

    stack_size = 1024;
    stack = calloc(stack_size, sizeof *stack);
    uint16_t *sp = stack + 12;

    // set some values
    stack[1] = 1;
    stack[10] = 10;
    stack[13] = 13;
    stack[104] = 104;

    assert_true(grow_stack(&sp));

    //things should be where we left them
    assert_int_equal(stack[1], 1);
    assert_int_equal(stack[10], 10);
    assert_int_equal(stack[13], 13);
    assert_int_equal(stack[104], 104);
    assert_int_equal(stack_size, 1024 + 256);

    stack[1060] = 1060;

    // sp should point to same point even if stack moved
    assert_int_equal(sp-stack, 12);

    // EXPECT: fail and set errno=ENOMEM
    stack_size = UINT16_MAX;
    
    assert_false(grow_stack(&sp));
    assert_int_equal(errno, ENOMEM);

    free(stack);
}

int main(void)
{
    const struct CMUnitTest tests[] = 
    {
        cmocka_unit_test(test_halt_inst),
        cmocka_unit_test(test_grow_stack),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
