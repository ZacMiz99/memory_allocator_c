#include "memalloc.h"

/// @brief Allocate memory
/// @param size Number of bytes to allocate
/// @return Pointer to the allocated memory
void *malloc(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;

    if (!size)
        return NULL;

    // Lock the mutex
    pthread_mutex_lock(&global_malloc_lock);
    header = get_free_block(size); // Check if there is a free block that can be used
    if (header)
    {
        header->s.is_free = 0;                     // Mark the block as used
        pthread_mutex_unlock(&global_malloc_lock); // Unlock the mutex
        return (void *)(header + 1);               // Return the pointer to the block
    }

    total_size = sizeof(header_t) + size; // Calculate the total size of the block
    block = sbrk(total_size);             // Increase the program break to allocate memory
    if (block == (void *)-1)
    { // Check if the allocation was successful
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL; // Return NULL if the allocation failed and free mutex
    }

    // Set header to the block, and initialize
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;

    if (!head)
        head = header; // If the head is NULL, set the head to the header
    if (tail)
        tail->s.next = header; // If the tail is not NULL, set the next block of
                               // the tail to the header

    tail = header;                             // Set the tail to the header
    pthread_mutex_unlock(&global_malloc_lock); // Unlock the mutex

    return (void *)(header + 1); // Return the pointer to the block
}

/// @brief Check if there is a free block that can be used without allocating
/// new memory
/// @param size Number of bytes for the block
/// @return Pointer to the free block, or NULL if there is no free block
header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    while (curr)
    {
        if (curr->s.size >= size && curr->s.is_free)
        {
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}

/// @brief Free memory
/// @param block Pointer to the memory block to free
void free(void *block)
{
    header_t *header, *tmp;
    void *program_break;

    // If block is NULL, return
    if (!block)
        return;

    // Lock the mutex
    pthread_mutex_lock(&global_malloc_lock);

    // Get the header of the block and the current program break
    header = (header_t *)block - 1;
    program_break = sbrk(0);

    // Check if the block is the last block
    if ((char *)block + header->s.size == program_break)
    {
        if (head == tail)
        {
            head = tail = NULL; // If the head and tail are the same, set them to NULL
        }
        else
        {
            tmp = head;
            while (tmp) // Find the block before the tail
            {
                if (tmp->s.next == tail)
                {
                    tmp->s.next = NULL; // Set the next block of the block before the tail to NULL
                    tail = tmp;         // Set the tail to the block before the tail
                }
                tmp = tmp->s.next;
            }
            sbrk(0 - sizeof(header_t) - header->s.size); // Decrease the program break
            pthread_mutex_unlock(&global_malloc_lock);   // Unlock the mutex
            return;
        }
    }

    header->s.is_free = 1;                     // Mark the block as free
    pthread_mutex_unlock(&global_malloc_lock); // Unlock the mutex
    return;
}

/// @brief Allocate memory and set it to zero
/// @param num Number of elements
/// @param nsize Size of each element
/// @return Pointer to the allocated memory
void *calloc(size_t num, size_t nsize)
{
    size_t size;
    void *block;
    if (!num || !nsize) // Check if num or nsize is 0
        return NULL;
    size = num * nsize;

    if (nsize != size / num) // Check for overflow
        return NULL;

    block = malloc(size); // Allocate memory

    if (!block)
        return NULL;

    memset(block, 0, size); // Set the memory to zero
    return block;
}

/// @brief Allocate memory and copy the content of the old block to the new block
/// @param block Pointer to the old block
/// @param size Size of the new block
/// @return Pointer to the new block
void *realloc(void *block, size_t size)
{
    header_t *header;
    void *ret;

    if (!block || !size)
        return malloc(size); // If block is NULL, allocate memory

    header = (header_t *)block - 1; // Get the header of the block

    if (header->s.size >= size)
        return block; // If the size of the block is greater than or equal to the new size, return the block

    ret = malloc(size); // Allocate memory with the new size

    if (ret)
    {
        memcpy(ret, block, header->s.size); // Copy the content of the old block to the new block
        free(block);                        // Free the old block
    }

    return ret;
}
