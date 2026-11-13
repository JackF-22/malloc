#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#define META_SIZE sizeof(MemRegion)
void *global_base = NULL;

struct MemRegion {
    size_t size;
    struct MemRegion* next;
    int free;  // 0 for used, 1 for free
} MemRegion;

int main(void) {
    printf("Hello, World!");

    return 0;
}

struct MemRegion *find_memory(struct MemRegion **last, size_t size) {
    struct MemRegion *current = global_base;
    while (current && !(current->free && current->size >= size)) {
        // Advance through memory to the next pointer
        *last = current;
        current = current->next;
    }
    return current;
}

struct MemRegion *request_space(struct MemRegion *last, size_t size) {
    // Ask for a pointer to the top of the heap
    struct MemRegion *block = sbrk(0);
    void *request = sbrk(size + META_SIZE); 

    if (request == (void*) -1) {
        return NULL;
    }
    if (last) {
        last->next = block;
    }

    block->size = size;
    block->next = NULL;
    block->free = 0;
    return block;
}

void *malloc_v2(size_t size) {
    struct MemRegion *new_mem;

    // Checks 
    if (size < 0) {
        return NULL;
    }

    if (!global_base) {
        new_mem = request_space(NULL, size);
        if (!new_mem) {
            return NULL;
        }
        // Initialize our base
        global_base = new_mem;
    }
    else {
        new_mem = find_memory(&global_base, size);
        if (!new_mem) {
            new_mem = request_space(global_base, size);
            if (!new_mem) {
                return NULL;
            }
        }
        else {
            new_mem->free = 0;
        }
    }
    
    return new_mem+1;
}

struct MemRegion *get_region_ptr(void *ptr) {
    return (struct MemRegion*)ptr - 1;
}

void free(void* ptr) {
    if (!ptr) {
        return;
    }
    
    struct MemRegion *mem_ptr = get_region_ptr(ptr);
    if (!mem_ptr || mem_ptr->free == 1) {
        return;
    }
    else {
        mem_ptr->free = 0;
        return;
    }
}

void *malloc_v1(size_t bytes) {
    // Pointer to the top of the heap
    void *ptr = sbrk(0);

    // Try assigning the number of bytes
    void *request = sbrk(bytes + META_SIZE);

    if (request == (void*) -1) {
        return NULL;
    }
    else {
        return ptr;
    }
}