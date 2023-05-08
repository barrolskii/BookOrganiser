#include "book.h"

book_t *init_book()
{
    book_t *book = calloc(1, sizeof(book_t));
    book->tags = dynamic_array_init();

    return book;
}

void free_book(void *ptr)
{
    book_t *book = ptr;
    dynamic_array_clear_user_data(book->tags, free);
    dynamic_array_free(book->tags);
    free(book);
}

cJSON *serialise_book(book_t *book)
{
    cJSON *json_book = cJSON_CreateObject();

    if (NULL == json_book)
        return NULL;

    cJSON_AddStringToObject(json_book, "name", book->name);

    cJSON *tag_array = cJSON_AddArrayToObject(json_book, "tags");
    for (size_t i = 0; i < book->tags->size; ++i)
    {
        cJSON *tag = cJSON_CreateString(book->tags->data[i]);
        cJSON_AddItemToArray(tag_array, tag);
    }

    cJSON_AddBoolToObject(json_book, "have read",   book->have_read);
    cJSON_AddBoolToObject(json_book, "in progress", book->in_prog);
    cJSON_AddBoolToObject(json_book, "to read",     book->to_read);

    return json_book;
}

book_t *deserialise_book(cJSON *book)
{
    cJSON *name      = cJSON_GetObjectItem(book, "name");
    cJSON *tags      = cJSON_GetObjectItem(book, "tags");
    cJSON *have_read = cJSON_GetObjectItem(book, "have read");
    cJSON *in_prog   = cJSON_GetObjectItem(book, "in progress");
    cJSON *to_read   = cJSON_GetObjectItem(book, "to read");

    book_t *new_book = init_book();

    memccpy(new_book->name, name->valuestring, 0, BOOK_NAME_MAX);

    cJSON *itr = NULL;
    cJSON_ArrayForEach(itr, tags)
    {
        char *tag = strdup(itr->valuestring);
        dynamic_array_add(new_book->tags, tag);
    }

    new_book->have_read = cJSON_IsTrue(have_read);
    new_book->in_prog = cJSON_IsTrue(in_prog);
    new_book->to_read = cJSON_IsTrue(to_read);

    return new_book;
}
