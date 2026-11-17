#ifndef MYMALLOC_H
#define MYMALLOC_H

#include <stddef.h>

#define META_SIZE sizeof(MemRegion)
#define BYTE_ALIGNMENT 8

struct MemRegion {
    size_t size;
    struct MemRegion* next;
    int free;  // 0 for used, 1 for free
} MemRegion;

void *malloc_func(size_t size);
void free_func(void *ptr);
void *calloc_func(size_t count, size_t size);
void *realloc_func(void *ptr, size_t size);
void heap_state(void);
void fragment_check(struct MemRegion *metadata, size_t size);
void check_alignment(size_t* size);

#endif