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

void OrganiseBooks(const char *path)
{
	DIR *dir;
	struct dirent *ent;

	if((dir = opendir(path)) != NULL)
	{
		printf(ANSI_COLOR_GREEN "Directory %s found\n" ANSI_COLOR_RESET, path);

		char name[128] = {0};

		// Print all regular files within the given directory
		while((ent = readdir(dir)) != NULL)
		{
			if(ent->d_type == DT_REG)
			{
				strncpy(name, ent->d_name, 128);
				printf("name: %s\n", name);

				char *extension = NULL;
				extension = strrchr(name, '.');

				// Skip files that have a NULL extension of if they are
				// greater than the size of the type member of the book struct
				if (extension == NULL) continue;
				if (strlen(extension) > 7) continue;

				DisplayCategories();
				enum Category category = SetCategory();

				// Move book to category directory
				char pathToMove[128];
				strncpy(pathToMove, path, 128);
				DebugPrintf(ANSI_COLOR_CYAN "ptm: %s\n" ANSI_COLOR_RESET,
															pathToMove);

				// Concatenate category directory based on the input
				// category from the SetCategory() call
				strcat(pathToMove, categories[category]);
				DebugPrintf(ANSI_COLOR_CYAN "ptm: %s\n" ANSI_COLOR_RESET,
															pathToMove);

				char mvCmd[256] = "mv ";

				// Concat the current file to the current path
				char file[128] = {0};
				strcat(file, path);
				strcat(file, name);
				DebugPrintf(ANSI_COLOR_CYAN "file %s\n" ANSI_COLOR_RESET, file);

				// Concat the file with its path to the move command
				strcat(mvCmd, file);
				strcat(mvCmd, " ");
				strcat(mvCmd, pathToMove);
				DebugPrintf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, mvCmd);

				strncpy(file, "", 128);
				system(mvCmd); 
			}
		}

		closedir(dir);
	}
	else
	{
		printf(ANSI_COLOR_RED "Could not open directory\n" ANSI_COLOR_RED);
	}
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

		OrganiseBooks(path);
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
