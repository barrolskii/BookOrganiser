#ifndef DYNAMIC_ARRAY_H_
#define DYNAMIC_ARRAY_H_

#include <stdlib.h>

#define ARRAY_STARTING_CAPACITY 8

typedef struct {
    void **data;
    size_t size;
    size_t capacity;
} dynamic_array;

dynamic_array *dynamic_array_init();
void dynamic_array_free(dynamic_array *array);
void dynamic_array_set_size(dynamic_array *array, size_t new_size);
int dynamic_array_add(dynamic_array *array, void *data);
void dynamic_array_clear(dynamic_array *array);
void dynamic_array_clear_user_data(dynamic_array *array, void (*free_func)(void *));
int dynamic_array_insert(dynamic_array *array, size_t index, void *data);

#endif // INCLUDE_DYNAMIC_ARRAY

#ifdef DYNAMIC_ARRAY_IMPLEMENTATION

dynamic_array *dynamic_array_init()
{
    dynamic_array *array = (dynamic_array*)calloc(1, sizeof(dynamic_array));
    array->capacity = ARRAY_STARTING_CAPACITY;
    array->data = (void**)calloc(ARRAY_STARTING_CAPACITY, sizeof(void*));

    return array;
}

void dynamic_array_free(dynamic_array *array)
{
    if (!array)
        return;

    if (array->data)
        free(array->data);

    free(array);
}

static void **grow_array(dynamic_array *array)
{
    array->capacity *= 1.5;
    array->data = (void**)realloc(array->data, array->capacity * sizeof(void*));

    return array->data;
}

void dynamic_array_set_size(dynamic_array *array, size_t new_size)
{
    if (new_size == array->size)
        return;

    array->size = new_size;
    array->capacity = new_size;
    array->data = (void**)realloc(array->data, array->capacity * sizeof(void*));
}

int dynamic_array_add(dynamic_array *array, void *data)
{
    if (!array)
        return -1;

    if (array->size == array->capacity)
        grow_array(array);

    array->data[array->size] = data;
    ++array->size;

    return 0;
}

void dynamic_array_clear(dynamic_array *array)
{
    for (size_t i = 0; i < array->size; ++i)
    {
        array->data[i] = NULL;
    }

    array->size = 0;
}

void dynamic_array_clear_user_data(dynamic_array *array, void (*free_func)(void *))
{
    for (size_t i = 0; i < array->size; ++i)
    {
        free_func(array->data[i]);
        array->data[i] = NULL;
    }

    array->size = 0;
}

int dynamic_array_insert(dynamic_array *array, size_t index, void *data)
{
    if (index >= array->size)
        return -1;

    array->data[index] = data;

    return 0;
}

#endif // DYNAMIC_ARRAY_IMPLEMENTATION
