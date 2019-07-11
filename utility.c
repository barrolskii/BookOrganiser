#include "utility.h"

void DisplayDebugMode()
{
	if (DEBUG_MODE)
		printf(ANSI_COLOR_GREEN "Debug mode enabled\n" ANSI_COLOR_RESET);
	else
		printf(ANSI_COLOR_RED "Debug mode disabled\n" ANSI_COLOR_RESET);
} 

int DebugPrintf(const char *format, ...)
{
#if DEBUG_MODE
	va_list arg;
	int done;

	va_start(arg, format);
	done = vfprintf(stdout, format, arg);
	va_end(arg);

	return done;
#endif
}

void PrintSize(void *obj)
{
	printf("Object size: %lu\n", sizeof(obj));
}

int CountFilesInDirectory(const char *path)
{
	int count = 0;

	DIR *dir;
	struct dirent *entry;

	if ((dir = opendir(path)) != NULL)
	{
		while((entry = readdir(dir)) != NULL)
		{
			switch(entry->d_type)
			{
				case DT_REG:	
					count++;
					break;
				case DT_UNKNOWN:
				case DT_FIFO:
				case DT_CHR:
				case DT_DIR:
				case DT_BLK:
				case DT_LNK:
				case DT_SOCK:
				case DT_WHT:
				default:
					break;
			}
		}
	}
	else
		return 0;

	return count;
}

int CheckDirectoryExists(const char *path)
{
	DIR *dir = NULL;

	if ((dir = opendir(path)) != NULL)
	{
		return 0;
	}
	else
	{
		return -1;
	}	
}

void ListDirectoryContents(const char *path)
{
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(path)) != NULL)
	{
		DebugPrintf(ANSI_COLOR_GREEN "Directory found\n" ANSI_COLOR_RESET);

		// Print all files and directories within directory
		while ((ent = readdir(dir)) != NULL)
		{
			printf("%s\n", ent->d_name);
		}
		closedir(dir);
	}
	else
	{
		printf(ANSI_COLOR_RED "Could not open directory\n" ANSI_COLOR_RESET);
	}
}
