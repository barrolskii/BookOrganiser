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
	for(int i = 0; i < TOTAL_CATEGORIES; i++)
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

void CheckIfSaveFileExists(const char *path)
{
	char saveFile[128] = {0};
	strcat(saveFile, path);
	strcat(saveFile, ".psvf");

	// Replace forward slashes with backslashes so the path can be saved
	// as a file
	int i = 0;
	while (saveFile[i] != '\0')
	{
		if (saveFile[i] == '/')
			saveFile[i] = '\\';

		i++;
	}

	DebugPrintf(ANSI_COLOR_CYAN "Save file: %s\n" ANSI_COLOR_RESET, saveFile);

	char savePath[256] = "Saves/";
	strcat(savePath, saveFile);
	DebugPrintf(ANSI_COLOR_CYAN "Save path: %s\n" ANSI_COLOR_RESET, savePath);

	if (access(savePath, F_OK) == 0)
		printf(ANSI_COLOR_GREEN "Save file found\n" ANSI_COLOR_RESET);
	else
	{
		printf(ANSI_COLOR_RED "No save file found\n" ANSI_COLOR_RESET);
		printf(ANSI_COLOR_GREEN "Creating save file for %s\n" ANSI_COLOR_RESET,
																		path);
	}

	char createFile[256] = "touch Saves/\"";

	strcat(createFile, saveFile);
	strcat(createFile, "\"");
	DebugPrintf(ANSI_COLOR_CYAN "createFile: %s\n" ANSI_COLOR_RESET,
														createFile);

	system(createFile);
}

char *GetSaveFile(const char *path)
{
	char saveFile[128] = "Saves/";
	strcat(saveFile, path);
	strcat(saveFile, ".psvf");

	DebugPrintf(ANSI_COLOR_CYAN "Save file: %s\n" ANSI_COLOR_RESET, saveFile);

	// Skip the first five characters in the save file string
	// We only want to change the remaining forward slash characters
	int i = 6; 
	while(saveFile[i] != '\0')
	{
		if(saveFile[i] == '/')
			saveFile[i] = '\\';

		i++;
	}

	char *savePath = NULL;

	if (access(saveFile, F_OK) == 0)
	{
		savePath = malloc(128);
		strncpy(savePath, saveFile, 128);

		DebugPrintf(ANSI_COLOR_GREEN "Save file %s found\n" ANSI_COLOR_RESET,
																	savePath);
	}
	else
	{
		printf(ANSI_COLOR_RED "Cant find %s\n" ANSI_COLOR_RESET, savePath);
	}

	return savePath;
}

void AddSaveDataToDatabase(const char *path, struct Database *database)
{
	if(access(path, F_OK) == 0)
	{
		DebugPrintf(ANSI_COLOR_GREEN "%s found\n" ANSI_COLOR_RESET, path);

		FILE *fp = fopen(path, "r+");
		char c[148] = {0};

		DebugPrintf(ANSI_COLOR_ORANGE "Reading from file\n" ANSI_COLOR_RESET);

		while(fgets(c, 128, fp) != NULL)
		{
			DebugPrintf(ANSI_COLOR_ORANGE "c: %s" ANSI_COLOR_RESET, c);

			struct Book *book = malloc(sizeof(struct Book));

			// Parse book data and add it to the database
			char *token = strtok(c, "|");

			strncpy(book->name, token, 128);
			DebugPrintf(ANSI_COLOR_ORANGE "c: %s\n" ANSI_COLOR_RESET, token);

			token = strtok(NULL, "|");
			strncpy(book->type, token, 8);
			DebugPrintf(ANSI_COLOR_ORANGE "c: %s\n" ANSI_COLOR_RESET, token);

			token = strtok(NULL, "|");
			book->haveRead = atoi(token);
			DebugPrintf(ANSI_COLOR_ORANGE "c: %s\n" ANSI_COLOR_RESET, token);

			token = strtok(NULL, "|");
			book->isInterested = atoi(token);
			DebugPrintf(ANSI_COLOR_ORANGE "c: %s\n" ANSI_COLOR_RESET, token);

			token = strtok(NULL, "|");
			book->category = SetCategoryFromValue(atoi(token));
			DebugPrintf(ANSI_COLOR_ORANGE "c: %s\n" ANSI_COLOR_RESET, token);

			token = strtok(NULL, "|");
			DebugPrintf(ANSI_COLOR_ORANGE "c: %s\n" ANSI_COLOR_RESET, token);

			Add(database, book);
		}

		fclose(fp);
	}
	else
	{
		printf(ANSI_COLOR_RED "Error: Couldn't find save file\n"
												ANSI_COLOR_RESET);
	}
}

void CheckDirectories(const char *path, struct Database *database)
{
	DebugPrintf(ANSI_COLOR_CYAN "Check dir: %s\n" ANSI_COLOR_RESET, path);

	char dirPath[128] = {0};
	char name[128]	  = {0};

	for(int i = 0; i < TOTAL_CATEGORIES; i++)
	{
		strncpy(dirPath, path, 128);
		strcat(dirPath, categories[i]);

		DebugPrintf(ANSI_COLOR_RED "dirPath: %s\n" ANSI_COLOR_RESET, dirPath);

		DIR *dir;
		struct dirent *ent;

		if((dir = opendir(dirPath)) != NULL)
		{
			while((ent = readdir(dir)) != NULL)
			{
				if(ent->d_type == DT_REG)
				{
					DebugPrintf(ANSI_COLOR_ORANGE "file %s\n" ANSI_COLOR_RESET,
																ent->d_name);

					strncpy(name, ent->d_name, 128);
					if(CheckBook(database, strtok(ent->d_name, ".")) != 0)
					{
						printf(ANSI_COLOR_GREEN "Adding book to database\n"
														ANSI_COLOR_RESET);

						struct Book *book = malloc(sizeof(struct Book));

						strncpy(book->type, strrchr(name, '.'), 8);
						strncpy(book->name, strtok(name, "."), 128);
						book->haveRead = 0;
						book->isInterested = 0;
						book->category = SetCategoryFromValue(i);

						DebugPrintf(ANSI_COLOR_GREEN "Adding %s\n"
									ANSI_COLOR_RESET, book->name);

						Add(database, book);
					}
					else
						DebugPrintf(ANSI_COLOR_RED "Book skipped\n"
												ANSI_COLOR_RESET);
				}
			}
		}

		strncpy(dirPath, " ", 128);
	}
}

void WriteDataToSaveFile(struct Database *database, const char *path)
{
	DebugPrintf(ANSI_COLOR_ORANGE "Path %s\n" ANSI_COLOR_RESET, path);
	DebugPrintf(ANSI_COLOR_ORANGE "Writing data to file" ANSI_COLOR_RESET);

	FILE *fp = fopen(path, "w+");

	struct ListNode *head = database->head;
	while(head)
	{
		DebugPrintf(ANSI_COLOR_ORANGE "Parsing data\n" ANSI_COLOR_RESET);

		char *parsedData = ParseBookDataForWriting(head->data);
		DebugPrintf(ANSI_COLOR_ORANGE "Parsed data: %s\n" ANSI_COLOR_RESET,
																parsedData);

		fputs(parsedData, fp);
		free(parsedData);

		head = head->next;
	}

	fclose(fp);
}

void DisplayMenu()
{
	const char *options[8] = {
		"Exit program",
		"Print all records",
		"Print all unread books",
		"Print all read books",
		"Print all interesed books",
		"Print all books of category",
		"Print all books of extension",
		"Print book by ID",
	};

	printf("======================================\n");

	for(int i = 0; i < 8; i++)
	{
		printf("| %-3i| %-30s|\n", i, options[i]);
	}

	printf("======================================\n");
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
		CheckIfSaveFileExists(path);
	}
	else
	{

		printf(ANSI_COLOR_RED "No files found in directory\n" ANSI_COLOR_RESET);
	}

	struct Database *bookDatabase = malloc(sizeof(struct Database));

	char *saveFile = GetSaveFile(path);
	AddSaveDataToDatabase(saveFile, bookDatabase);


	// Maybe change to check category directories exist
	CheckDirectories(path, bookDatabase);

	unsigned int input = 1;

	while(input)
	{
		DisplayMenu();

		scanf("%d", &input);

		switch(input)
		{
			case 0:
				printf("Exiting program. Goodbye\n");
				break;
			case 1:
				PrintAllRecords(bookDatabase->head);
				break;
			case 2:
				PrintAllUnreadBooks(bookDatabase->head);
				break;
			case 3:
				PrintAllReadBooks(bookDatabase->head);
				break;
			case 4:
				PrintAllInterestedBooks(bookDatabase->head);
				break;
			case 5:
				PrintAllBooksOfCategory(bookDatabase->head);
				break;
			case 6:
				PrintAllBooksOfExtension(bookDatabase->head);
				break;
			case 7:
				PrintBookById(bookDatabase->head);
				break;
			default:
				printf("Please choose a valid option\n");
		}
	}

	WriteDataToSaveFile(bookDatabase, saveFile);

	DeleteDatabaseData(bookDatabase);

	free(bookDatabase);
	free(saveFile);

	return 0;
}
