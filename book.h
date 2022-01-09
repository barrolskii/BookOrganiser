#ifndef BOOK_H_
#define BOOK_H_

#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    char *tags;
    unsigned char have_read : 1;
    unsigned char in_prog : 1;
    unsigned char to_read : 1;
} book_t;

struct _book_node {
    book_t *book;
    struct _book_node *next;
};

typedef struct _book_node book_node_t;


book_t *init_book(const char *name, const char *tags);
void    free_book(book_t *book);

book_node_t *init_book_node(const char *name, const char *tags);
void         free_node(book_node_t *node);

#endif // BOOK_H_
