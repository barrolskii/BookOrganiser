#ifndef BOOK_H_
#define BOOK_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "cJSON.h"
#include "dynamic_array.h"

#define TAGS_MAX 1024
#define BOOK_NAME_MAX 256 // TODO: Use the linux max name length here

typedef struct {
    char name[BOOK_NAME_MAX];
    //char tags[TAGS_MAX];
    dynamic_array *tags;
    bool have_read;
    bool in_prog;
    bool to_read;
} book_t;

book_t *init_book();
void free_book(void *ptr);

cJSON *serialise_book(book_t *book);
book_t *deserialise_book(cJSON *book);

#endif /* BOOK_H_ */
