#ifdef FILE_UTILS_IMPLEMENTATION
#undef FILE_UTILS_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t file_count_lines(const char *path)
{
    size_t count = 0;

    FILE *fp = fopen(path, "r");

    if (fp == NULL)
        return -1;

    int c;

    while ((c = fgetc(fp)) != EOF)
    {
        if (c == '\n')
            ++count;
    }

    fclose(fp);

    return count;
}

char *file_as_str(const char *path)
{
    FILE *fp = fopen(path, "r");

    if (fp == NULL)
        return NULL;

    fseek(fp, 0, SEEK_END);
    int bytes = ftell(fp);

    char *contents = (char*)calloc(bytes + 1, sizeof(char));

    rewind(fp);
    (void)!fread(contents, sizeof(char), bytes, fp);

    fclose(fp);

    return contents;
}

/*
 * Returns file as array of strings
 * Array is count is line count of file + 1
 * The last element in the array is NULL
 */
char **file_as_str_arr(const char *path)
{
    size_t lines = file_count_lines(path);

    if (lines == (size_t)-1)
        return NULL;

    FILE *fp = fopen(path, "r");

    if (fp == NULL)
        return NULL;


    char **contents = (char**)calloc(lines + 1, sizeof(char*));
    char *buffer = NULL;
    size_t buf_sz = 4096;

    for (size_t i = 0; i < lines; ++i)
    {
        int size = getline(&buffer, &buf_sz, fp);
        contents[i] = (char*)calloc(size + 1, sizeof(char));

        memcpy(contents[i], buffer, size);
    }


    free(buffer);
    fclose(fp);

    return contents;
}

#endif /* FILE_UTILS_IMPLEMENTATION */

#if !defined(INCLUDE_FILE_UTILS_H)
#define INCLUDE_FILE_UTILS_H

#include <stddef.h>

size_t file_count_lines(const char *path);
char *file_as_str(const char *path);
char **file_as_str_arr(const char *path);

#endif /* INCLUDE_FILE_UTILS_H */
