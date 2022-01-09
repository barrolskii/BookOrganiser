#ifndef DEFINES_H_
#define DEFINES_H_

#define ENTER 10
#define ESCAPE_KEY 27
#define MAX_LENGTH NAME_MAX + PATH_MAX

#define VERSION        "2.0.1"
#define BOOK_DATA_FILE "books.csv"

#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

#define FREE_2D_ARR(arr, size)     \
    for (int i = 0; i < size; i++) \
        free(arr[i]);              \
    free(arr);                     \


#endif // DEFINES_H_
