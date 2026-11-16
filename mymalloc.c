#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "mymalloc.h"

static void *global_base = NULL;

struct MemRegion *find_memory(struct MemRegion **last, size_t size) {
    struct MemRegion *current = global_base;
    while (current && !(current->free && current->size >= size)) {
        // Advance through memory to the next pointer
        *last = current;
        current = current->next;
    }
    fragment_check(current, size);
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

void *malloc_func(size_t size) {
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

void free_func(void* ptr) {
    if (!ptr) {
        return;
    }
    
    struct MemRegion *mem_ptr = get_region_ptr(ptr);
    if (!mem_ptr || mem_ptr->free == 1) {
        return;
    }
    else {
        mem_ptr->free = 1;
        return;
    }
}

void *realloc_func(void *ptr, size_t size) {
    if (!ptr) {
        return malloc_func(size);
    }

    struct MemRegion *mem_ptr = get_region_ptr(ptr);

    // We have enough size and can shrink and return same start
    if (size < mem_ptr->size) {
        mem_ptr->size = size;
        return ptr;
    }
    
    // We need to reallocate
    void *new_ptr;
    new_ptr = malloc_func(size);
    if (!new_ptr) {
        return NULL;
    }

    // Copy memory over to the new destination with memcpy
    memcpy(new_ptr, ptr, mem_ptr->size);
    free_func(ptr);
    return new_ptr;
}

void *calloc_func(size_t elts, size_t elt_size) {
    void *mem = malloc_func(elts * elt_size);
    memset(mem, 0, elts * elt_size);
    return mem;
}


/*
What if we free a block of 1000 bytes, then use it for a 100 byte request
We end up allocating all 1000 bytes for this request because it's
defined by the metadata. Let's create fragmentation to avoid this.
*/
void fragment_check(struct MemRegion *metadata, size_t requested_size) {
    if (!metadata) {
        return;
    }

    // Let's take a look at metadata to see the size
    if (metadata->size - requested_size <= META_SIZE) {
        return;
    }
    else {
        // Change the metadata size to requested and then we need
        // to create an updated piece of metadata

        struct MemRegion *new_data = (struct MemRegion*)((char*) metadata + META_SIZE + requested_size);
        new_data->size = metadata->size - (requested_size + META_SIZE);
        new_data->next = metadata->next;
        new_data->free = 1;

        metadata->next = new_data;
        metadata->size = requested_size;
    }
}


// ============== EXTRA ADDITIONS ===============
void heap_state() {
    if (!global_base) {
        printf("Heap is empty\n");
    }
    else {
        struct MemRegion *ptr = global_base;
        int num_block = 1;
        while (ptr != NULL) {
            printf("Block: %d | Size: %d | Free: %s\n", 
                num_block++, ptr->size, ptr->free == 0 ? "No" : "Yes");
            ptr = ptr->next;
        }
    }
}


