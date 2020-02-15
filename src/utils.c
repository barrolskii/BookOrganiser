#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "utils.h"
#include "terminalColors.h"

/*
 * Print utility functions
 */

unsigned int get_longest_strlen(char **arr, int arr_len)
{
	unsigned int length = 0;
	unsigned int curr_len = 0;

	for (int i = 0; i < arr_len; i++)
	{
		curr_len = strlen(arr[i]);

		if (curr_len > length)
			length = curr_len;
	}

	return length;
}

void print_str_array(char **arr, int len)
{
	unsigned int length = get_longest_strlen(arr, len);

	for (int i = 0; i < len; i++)
		printf("| %-2d: %-*s |\n", i + 1, length, arr[i]);
}

void print_border(void (*fun)(char **, int), char **arr, int arr_len)
{
	// Add 9 to the length for the extra output padding for the table
	// display
	int len = get_longest_strlen(arr, arr_len) + 9;

	char border[len]; 

	memset(border, '=', len * sizeof(char));
	border[len - 1] = '\0'; // Set the last character to the null terminator
							// to stop garbage characters from being displayed
							// in the stdout

	printf("%s\n", border);

	fun(arr, arr_len);

	printf("%s\n", border);

	memset(border, 0, sizeof(border));
}


/*
 * Directory utility functions
 */

int check_directory_exists(char *dir_name)
{
	DIR *dir = NULL;

	if ((dir = opendir(dir_name)) != NULL)
	{
		closedir(dir);
		return 0;
	}

	closedir(dir);
	return -1;
}

int create_directory(char *name)
{
	printf(ANSI_COLOR_GREEN "Creating directory %s\n" ANSI_COLOR_RESET, name);

	char create_cmd[64] = "mkdir ";
	strcat(create_cmd, name);

	// Shouldn't really be using the system function for this but it's a
	// quick and easy way to create a directory right now
	system(create_cmd);

	return 0;
}

unsigned int count_files_in_directory(char *path)
{
	unsigned int count = 0;

	DIR *dir = NULL;
	struct dirent *entry = NULL;

	if ((dir = opendir(path)) != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			switch (entry->d_type)
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
		} // End while
	} // End if

	closedir(dir);

	return count;
}
