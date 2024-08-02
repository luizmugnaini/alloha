/// Single compilation unit comprising the whole suite of tests for the Alloha library.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#include "../src/all.c"

#define ALLOHA_TEST_NO_MAIN
#include "test_arena.c"
#include "test_stack.c"

int main(void) {
    test_arena();
    test_stack();
    return 0;
}
