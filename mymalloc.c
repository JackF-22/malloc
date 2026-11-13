#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

void *global_base = NULL;

struct MemRegion {
    size_t size;
    struct MemRegion* next;
    int free;  // 0 for free, 1 for used
} MemRegion;

int main(void) {
    printf("Hello, World!");

    return 0;
}

struct MemRegion *find_memory(struct MemRegion **last, size_t size) {
    struct MemRegion *current = global_base;
    while (current && !(current->free && current->size >= size)) {
        
    }
}

void *malloc_v2(size_t size) {
    // Create a memory region struct
    struct MemRegion *mem = malloc(sizeof(MemRegion));
    mem->size = size;
    mem->next = NULL;
    mem->free = 0;
}

void *malloc_v1(size_t bytes) {
    // Pointer to the top of the heap
    void *ptr = sbrk(0);

    // Try assigning the number of bytes
    void *request = sbrk(bytes);

    if (request == (void*) -1) {
        return NULL;
    }
    else {
        return ptr;
    }
}