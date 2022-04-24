#include "dir_utils.h"

int dir_exists(char *path)
{
    DIR *dir = NULL;

    if ((dir = opendir(path)) != NULL)
    {
        closedir(dir);
        return 1;
    }

    return 0;
}

unsigned dir_count_files(char *path)
{
    unsigned count = 0;

    DIR *dir = NULL;
    struct dirent *entry = NULL;

    if ((dir = opendir(path)) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_REG)
                ++count;
        }
    }

    closedir(dir);

    return count;
}

char **dir_get_files(char *path)
{
    int i = 0;
    unsigned file_count = dir_count_files(path);

    char **files = calloc(file_count, sizeof(char*));
    for (unsigned i = 0; i < file_count; i++)
    {
        files[i] = calloc(NAME_MAX, sizeof(char));
    }

    DIR *dir = NULL;
    struct dirent *entry = NULL;

    if ((dir = opendir(path)) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type != DT_REG)
                continue;

            memcpy(files[i], entry->d_name, NAME_MAX);
            i++;
        }
    }

    closedir(dir);

    return files;
}
