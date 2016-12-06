#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "inst.h"

extern void exec(instruction i);
extern int run;

static void test_halt_inst(void **state)
{
    (void) state;
    run = 1;
    exec(halt);

    assert_int_equal(run, 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = 
    {
        cmocka_unit_test(test_halt_inst),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
