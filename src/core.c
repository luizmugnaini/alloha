#include <alloha/core.h>

int is_power_of_two(int x) {
    return x > 0 && !(x & (x - 1));
}
