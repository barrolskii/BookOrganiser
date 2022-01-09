#ifndef DIR_UTILS_H_
#define DIR_UTILS_H_

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int        dir_exists(char *path);
unsigned   dir_count_files(char *path);
char     **dir_get_files(char *path);

#endif // DIR_UTILS_H_
