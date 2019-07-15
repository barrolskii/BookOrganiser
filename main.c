#include <stdio.h>
#include <unistd.h>

#include "terminalColors.h"
#include "utility.h"
#include "book.h"
#include "database.h"

void Init(const char *path)
{
	DebugPrintf(ANSI_COLOR_RED "Path: %s\n", path);

	char makeDir[128] = "mkdir ";
	strcat(makeDir, path);
	DebugPrintf(ANSI_COLOR_RED "makeDir: %s\n" ANSI_COLOR_RESET, makeDir);

	char pathCheck[128];
	strncpy(pathCheck, path, 128);

	const char *dir = NULL;

	// Check if enum categories exitst and create them if they don't
	for(int i = 0; i < 6; i++)
	{
		strcat(pathCheck, categories[i]);

		if(access(pathCheck, F_OK) == 0)
		{
			printf(ANSI_COLOR_RED "%s exists\n" ANSI_COLOR_RESET,
													categories[i]);
		}
		else
		{
			printf(ANSI_COLOR_GREEN "Creating %s directory\n" ANSI_COLOR_RESET,
																categories[i]);

			dir = categories[i];
			strcat(makeDir, dir);
			system(makeDir);
		}

		strncpy(makeDir, "mkdir ", 128);
		strcat(makeDir, path);

		strncpy(pathCheck, path, 128);
	}

	// Create save directory if one is not found
	if (access("Saves", F_OK) == 0)
	{
		printf(ANSI_COLOR_GREEN "Saves directory found\n" ANSI_COLOR_RESET);
	}
	else
	{
		printf(ANSI_COLOR_RED "No saves directory found\n" ANSI_COLOR_RESET);
		printf(ANSI_COLOR_GREEN "Creating saves directory\n" ANSI_COLOR_RESET);
		system("mkdir Saves");
	}

	DisplayDebugMode();
}

void DisplayMenu()
{
	printf("Display menu\n");
}

int main(int argc, char **argv)
{
	char path[128] = {0};
	
	printf("Enter a directory to sort\n");
	scanf("%s", path);

	if (CheckDirectoryExists(path) != 0)
	{
		printf(ANSI_COLOR_RED "Directory not found\n" ANSI_COLOR_RESET);
		return 0;
	}
	else
	{
		printf(ANSI_COLOR_GREEN "Directory found\n" ANSI_COLOR_RESET);
		Init(path);
	}

	unsigned int fileCount = CountFilesInDirectory(path);

	if (fileCount > 0)
	{
		printf(ANSI_COLOR_GREEN "%d files found\n" ANSI_COLOR_RESET, fileCount);

		//OrganiseBooks(path);
		//CheckIfSaveFileExists(path);
	}
	else
	{

		printf(ANSI_COLOR_RED "No files found in directory\n" ANSI_COLOR_RESET);
	}

	//struct Database *bookDatabase = malloc(sizeof(Database));

	//char *saveFile = GetSaveFile(path);
	//AddSaveDataToDatabase(saveFile, bookDatabase);

	// Maybe change to check category directories exist
	//CheckDirectories(path);

	unsigned int input = 1;
	//struct Node *head = bookDatabase->head;

	while(input != 0)
	{
		DisplayMenu();

		scanf("%d", &input);

		switch(input)
		{
			case 0:
				printf("Exiting program. Goodbye\n");
				break;
			case 1:
				// Print all records
				break;
			case 2:
				// Print unread books
				break;
			case 3:
				// Print all read books
				break;
			case 4:
				// Print all interested books
				break;
			case 5:
				// Print all books of input category
				break;
			case 6:
				// Print all books of extension
				break;
			default:
				printf("Please choose a valid option\n");
		}
	}

	//WriteDataToSaveFile(bookDatabase, saveFile);

	// free(bookDatabase);
	// free(saveFile);

	return 0;
}
