#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdarg.h>

#include "terminalColors.h"


#define DEBUG_MODE 0

void DisplayDebugMode();
int DebugPrintf(const char *format, ...);
void PrintSize(void *obj);

int CountFilesInDirectory(const char *path);
int CheckDirectoryExists(const char *path);
void ListDirectoryContents(const char *path);

#endif // UTILITY_H
