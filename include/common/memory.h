#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

/**
 * Safe memory allocation with null check
 * Returns NULL on failure
 */
void *safe_malloc(size_t size);

/**
 * Safe calloc with null check
 * Returns NULL on failure
 */
void *safe_calloc(size_t count, size_t size);

/**
 * Safe realloc with overflow protection
 * Returns NULL on failure (original pointer remains valid)
 */
void *safe_realloc(void *ptr, size_t new_size);

/**
 * Safe free (null-safe)
 */
void safe_free(void *ptr);

/**
 * Dynamic array structure
 */
typedef struct {
    void *data;
    size_t size;        // Current number of elements
    size_t capacity;    // Allocated capacity
    size_t elem_size;   // Size of each element
} darray_t;

/**
 * Create a new dynamic array
 */
darray_t *darray_create(size_t elem_size, size_t initial_capacity);

/**
 * Append an element to the array
 * Returns 0 on success, -1 on failure
 */
int darray_append(darray_t *arr, const void *elem);

/**
 * Get element at index (returns NULL if out of bounds)
 */
void *darray_get(darray_t *arr, size_t index);

/**
 * Clear all elements (keeps capacity)
 */
void darray_clear(darray_t *arr);

/**
 * Free the dynamic array
 */
void darray_free(darray_t *arr);

/**
 * Reserve capacity (grow if needed)
 */
int darray_reserve(darray_t *arr, size_t new_capacity);

#endif // MEMORY_H
