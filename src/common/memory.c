#include "common/memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void *safe_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    return malloc(size);
}

void *safe_calloc(size_t count, size_t size) {
    if (count == 0 || size == 0) {
        return NULL;
    }
    return calloc(count, size);
}

void *safe_realloc(void *ptr, size_t new_size) {
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, new_size);
}

void safe_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

darray_t *darray_create(size_t elem_size, size_t initial_capacity) {
    if (elem_size == 0) {
        return NULL;
    }
    
    darray_t *arr = safe_malloc(sizeof(darray_t));
    if (!arr) {
        return NULL;
    }
    
    if (initial_capacity == 0) {
        initial_capacity = 16; // Default capacity
    }
    
    arr->data = safe_malloc(elem_size * initial_capacity);
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    
    arr->size = 0;
    arr->capacity = initial_capacity;
    arr->elem_size = elem_size;
    
    return arr;
}

int darray_reserve(darray_t *arr, size_t new_capacity) {
    if (!arr || new_capacity <= arr->capacity) {
        return 0;
    }
    
    // Check for overflow
    if (new_capacity > SIZE_MAX / arr->elem_size) {
        return -1;
    }
    
    void *new_data = safe_realloc(arr->data, arr->elem_size * new_capacity);
    if (!new_data) {
        return -1;
    }
    
    arr->data = new_data;
    arr->capacity = new_capacity;
    return 0;
}

int darray_append(darray_t *arr, const void *elem) {
    if (!arr || !elem) {
        return -1;
    }
    
    // Grow if needed (double capacity)
    if (arr->size >= arr->capacity) {
        size_t new_capacity = arr->capacity * 2;
        if (new_capacity < arr->capacity) { // Overflow check
            return -1;
        }
        
        if (darray_reserve(arr, new_capacity) != 0) {
            return -1;
        }
    }
    
    // Copy element
    char *dest = (char *)arr->data + (arr->size * arr->elem_size);
    memcpy(dest, elem, arr->elem_size);
    arr->size++;
    
    return 0;
}

void *darray_get(darray_t *arr, size_t index) {
    if (!arr || index >= arr->size) {
        return NULL;
    }
    
    return (char *)arr->data + (index * arr->elem_size);
}

void darray_clear(darray_t *arr) {
    if (arr) {
        arr->size = 0;
    }
}

void darray_free(darray_t *arr) {
    if (arr) {
        safe_free(arr->data);
        free(arr);
    }
}
