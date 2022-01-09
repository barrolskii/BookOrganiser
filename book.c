#include "book.h"

book_t *init_book(const char *name, const char *tags)
{
    book_t *new_book = calloc(1, sizeof(book_t));

    new_book->name = malloc( (sizeof(char) * strlen(name)) + 1);
    strcpy(new_book->name, name);
    new_book->name[strlen(new_book->name)] = '\0';

    new_book->tags = malloc( (sizeof(char) * strlen(tags)) + 1);
    strcpy(new_book->tags, tags);
    new_book->tags[strlen(new_book->tags)] = '\0';

    /* This fixes an issue with the form character padding       */
    /* I've tried changing it to be a NULL termination character */
    /* but that doesn't work. So we just find the last valid     */
    /* character and put a NULL terminator after that            */
    for (int i = strlen(new_book->tags); i > 0; i--)
    {
        if ((new_book->tags[i] >= 33) && (new_book->tags[i] <= 126))
        {
            new_book->tags[i + 1] = '\0';
            break;
        }
    }

    return new_book;
}

void free_book(book_t *book)
{
    free(book->name);
    free(book->tags);
    free(book);
}

book_node_t *init_book_node(const char *name, const char *tags)
{
    book_node_t *new_node = calloc(1, sizeof(book_t));
    new_node->book = init_book(name, tags);

    return new_node;
}

void free_node(book_node_t *node)
{
    free_book(node->book);
    free(node);
}
