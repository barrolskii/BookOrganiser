#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FNV_OFFSET_BASIS 14695981039346656037u
#define FNV_PRIME 1099511628211u

#define MAX_TABLE_LOAD 0.60
#define CAPACITY_MULTIPLIER 2.0

#define KEY_LENGTH 128

typedef struct {
    void *value;
    char key[KEY_LENGTH];
} hash_item_t;

typedef struct {
    hash_item_t **items;
    size_t count;
    size_t capacity;
} hash_table_t;

hash_table_t *hash_table_init();
void hash_table_free(hash_table_t *table);

int hash_table_add(hash_table_t *table, char *key, void *value);
hash_item_t *hash_table_get(hash_table_t *table, char *key);
hash_item_t *hash_table_set(hash_table_t *table, char *key, void *value);
void hash_table_remove(hash_table_t *table, char *key);

void hash_table_free_item(hash_table_t *table, char *key, void (*free_func)(void *));
void hash_table_clear_user_data(hash_table_t *table, void (*free_func)(void *));

#endif // INCLUDE_HASHTABLE

#ifdef HASHTABLE_IMPLEMENTATION

static hash_item_t *new_item(char *key, void *value)
{
    hash_item_t *item = calloc(1, sizeof(hash_item_t));
    memccpy(item->key, key, 0, KEY_LENGTH);
    item->value = value;

    return item;
}

static uint64_t fnv1a_hash(char *key, size_t table_size)
{
    uint64_t hash = FNV_OFFSET_BASIS;

    for (size_t i = 0; i < strlen(key); ++i)
    {
        hash ^= key[i];
        hash *= FNV_PRIME;
    }

    /* Modulo optimisation. Explanation on this at the bottom of this file */
    return hash & (table_size - 1);
}

static void grow_table(hash_table_t *table, size_t size)
{
    hash_item_t **items = calloc(size, sizeof(hash_item_t*));;

    for (size_t i = 0; i < table->capacity; ++i)
    {
        if (!table->items[i])
            continue;

        uint64_t index = fnv1a_hash(table->items[i]->key, table->capacity);

        while (items[index] != NULL)
            index = (index + 1) % table->capacity;

        items[index] = table->items[i];
    }

    free(table->items);

    table->items = items;
    table->capacity = size;
}

hash_table_t *hash_table_init()
{
    hash_table_t *table = calloc(1, sizeof(hash_table_t));
    table->items = calloc(8, sizeof(hash_item_t*));
    table->capacity = 8;

    return table;
}

void hash_table_free(hash_table_t *table)
{
    if (!table)
        return;

    for (size_t i = 0; i < table->capacity; ++i)
    {
        if (table->items[i])
            free(table->items[i]);
    }

    if (table->items)
        free(table->items);

    free(table);
}

int hash_table_add(hash_table_t *table, char *key, void *value)
{
    if (table->count + 1 > table->capacity * MAX_TABLE_LOAD)
    {
         int new_capacity = table->capacity * CAPACITY_MULTIPLIER;
         grow_table(table, new_capacity);
    }

    uint64_t index = fnv1a_hash(key, table->capacity);

    if (table->items[index] != NULL)
    {
        while(table->items[index] != NULL)
        {
            index = (index + 1) & (table->capacity - 1);
        }
    }

    hash_item_t *item = new_item(key, value);
    table->items[index] = item;
    ++table->count;


    return index;
}

hash_item_t *hash_table_get(hash_table_t *table, char *key)
{
    uint64_t index = fnv1a_hash(key, table->capacity);

    if (!table->items[index])
        return NULL;

    while (table->items[index] != NULL)
    {
        if (strcmp(table->items[index]->key, key) == 0)
            return table->items[index];

        index = (index + 1) & (table->capacity - 1);
    }

    return NULL;
}

hash_item_t *hash_table_set(hash_table_t *table, char *key, void *value)
{
    hash_item_t *item = hash_table_get(table, key);

    if (!item)
        return item;

    item->value = value;
    return item;
}

void hash_table_remove(hash_table_t *table, char *key)
{
    hash_item_t *item = hash_table_get(table, key);

    if (!item)
        return;

    memset(item->key, 0, KEY_LENGTH);
    item->value = NULL;
    --table->count;
}

void hash_table_free_item(hash_table_t *table, char *key, void (*free_func)(void *))
{
    hash_item_t *item = hash_table_get(table, key);

    if (!item)
        return;

    free_func(item->value);
}

void hash_table_clear_user_data(hash_table_t *table, void (*free_func)(void *))
{
    for (size_t i = 0; i < table->capacity; ++i)
    {
        if (table->items[i])
            free_func(table->items[i]->value);
    }
}

/*
**
** Modulo Optimisation
**
** The modulo operator is computationally expensive. To optimise this we can use some
** bit trickery to speed this up. If for example we wanted to calculate 229 % 64:
**
** 229 - 1110 0101
** %
** 64  - 0100 0000
** =
** 37  - 0010 0101
**
** We can see that the dividend is 229 with the 2 highest bits cleared. If we bitwise and
** 229 with 63 we can get the same answer:
**
** 229 - 1110 0101
** &
** 63  - 0011 1111
** =
** 37  - 0010 0101
**
** Put simply a number modulo a prime can be calculated by using a bitwise and with the number
** against a power of 2 minus 1. This only works with the hashtable as the capacity is increased
** by 2 each time it grows. If the capacity modifier wasn't a power of 2 then this won't work.
**
** I can't take credit for this. When researching hashtables I came across the Crafting
** Interpreters book and in the optimisation chapter goes over this (The explanation is better
** here too so if I didn't explain it clearly this link should help):
** https://craftinginterpreters.com/optimization.html#faster-hash-table-probing
*/

#endif // HASHTABLE_IMPLEMENTATION
