#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "mymalloc.h"

static void *global_base = NULL;


/**
 * @get_region_ptr Turns a pointer to memory into a pointer to its metadata
 * 
 * @param ptr  Pointer to a region in memory
 * @return Pointer to that region's metadata block
 */
struct MemRegion *get_region_ptr(void *ptr) {
    return (struct MemRegion*)ptr - 1;
}


/**
 * @find_memory Traverses our array of metadata blocks searching for free memory
 * 
 * @param mem_arr Pointer to an array of metadata blocks of previously defined regions
 * @param size The number of bytes we are looking to allocate
 * @return A pointer to a metadata block in memory
 */
struct MemRegion *find_memory(struct MemRegion **mem_arr, size_t size) {
    struct MemRegion *current = global_base;
    while (current && !(current->free && current->size >= size)) {
        // Advance through memory to the next pointer
        *mem_arr = current;
        current = current->next;
    }
    fragment_check(current, size);
    return current;
}


/**
 * @request_space Utilizes the sbrk function to get a pointer on the heap
 * 
 * @param last A pointer to the base of memory (global_base)
 * @param size The number of bytes we need to allocate
 * @return A pointer to the metadata of the new region
 */
struct MemRegion *request_space(struct MemRegion *base, size_t size) {
    // sbrk changes the breakpoint of the heap
    // struct MemRegion *block = sbrk(0);
    void *request = sbrk(size + META_SIZE); 
    struct MemRegion *metadata = get_region_ptr(request);

    if (request == (void*) -1) {
        return NULL;
    }
    if (base) {
        base->next = request;
    }

    metadata->size = size;
    metadata->next = NULL;
    metadata->prev = base;
    metadata->free = 0;
    return metadata;
}


/**
 * @malloc_func Locate a region of memory that is available to use
 * 
 * @param size  The number of bytes the client desires
 * @return Pointer to the available region of memory (starting at the metadata)
 */
void *malloc_func(size_t size) {
    struct MemRegion *new_mem;

    // Checks 
    if (size < 0) {
        return NULL;
    }
    check_alignment(&size);

    if (!global_base) {
        new_mem = request_space(NULL, size);
        if (!new_mem) {
            return NULL;
        }
        // Initialize our base
        global_base = new_mem;
    }
    else {
        struct MemRegion *base = global_base;
        new_mem = find_memory(&base, size);
        if (!new_mem) {
            new_mem = request_space(base, size);
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


/**
 * @free_func Free a region of memory for future use
 * 
 * @param ptr A pointer to a region in memory
 */
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


/**
 * @calloc_func Initializes memory with a starting value of 0
 * 
 * @param elts  The number of elements we are initializing
 * @param elt_size  The size in bytes of each element
 * @return Pointer to this block of memorya
 */
void *calloc_func(size_t elts, size_t elt_size) {
    void *mem = malloc_func(elts * elt_size);
    memset(mem, 0, elts * elt_size);
    return mem;
}


/*
What if we free a block of 1000 bytes, then use it for a 100 byte request
We end up allocating all 1000 bytes for this request because it's
defined by the metadata. Let's create fragmentation to avoid this.

Original Setup:
-------------------------
| metadata | 1000 Bytes |
-------------------------

New Setup:
------------------------------------------------------
| metadata | 100 Bytes | new_data | 900 Bytes (Free) |
------------------------------------------------------
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

        // Check byte alignment
        check_alignment(&requested_size);

        struct MemRegion *new_data = (struct MemRegion*)((char*) metadata + META_SIZE + requested_size);
        new_data->size = metadata->size - (requested_size + META_SIZE);
        new_data->next = metadata->next;
        new_data->prev = metadata;
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

void check_alignment(size_t* size) {
    if (*size % BYTE_ALIGNMENT != 0) {
        *size += (BYTE_ALIGNMENT - (*size % BYTE_ALIGNMENT));
    }
}