#pragma once

#include <pthread.h>
#include <string.h>
#include <unistd.h>

typedef char ALIGN[16];
union header {
    // Header for free blocks
    struct
    {
        size_t size;        // Size of the block
        unsigned is_free;   // Flag to indicate if the block is free
        union header *next; // Pointer to the next free block
    } s;
    // Force the block to be aligned in a 16 byte boundary
    ALIGN stub;
};

typedef union header header_t;

pthread_mutex_t global_malloc_lock;

header_t *head, *tail;

header_t *get_free_block(size_t size);

void *malloc(size_t size);

void free(void *block);

void *calloc(size_t num, size_t nsize);

void *realloc(void *block, size_t size);