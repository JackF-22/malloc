#include <stdio.h>
#include <string.h>
#include "mymalloc.h"

int main(void)
{
    heap_state();

    // Test allocating memory
    int *num_arr = malloc_func(100 * sizeof(int));
    for (int i = 0; i < 10; i++) {
        *(num_arr) = i;
    }
    heap_state();

    // Test that it has been freed
    free_func(num_arr);
    heap_state();

    // Test allocating less memory than the block (fragmentation)
    num_arr = malloc_func(5 * sizeof(int));
    for (int i = 0; i < 10; i++) {
        *(num_arr) = i;
    }
    heap_state();
    free_func(num_arr);

    // Test block merge after previous free
    heap_state();

    return 0;
}
