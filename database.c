#include "database.h"

void PrintAllRecords(struct ListNode *head)
{
	struct ListNode *curr = head;

	while(curr)
	{
		PrintBookData(curr->data);

		curr = curr->next;
	}
}

void PrintAllBooksOfExtension(struct ListNode *head)
{
	struct ListNode *curr = head;
	char ext[8] = {0};

	printf("Enter extension\n");
	scanf("%s", ext);

	char border[87] = {[0 ... 86] = '='};
	char line[87] = {[0 ... 86] = '-'};

	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);


	printf("| %-3s | %-30s | %-8s | %-5s | %-11s | %-11s |\n",
				"id", "name", "type", "Read", "Interested", "category");

	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, line);

	while(curr)
	{
		if(strcmp(ext, curr->data->type) == 0)
			PrintBookData(curr->data);

		curr = curr->next;
	}

	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);
}

void PrintAllUnreadBooks(struct ListNode *head)
{
	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->haveRead == 0)
			PrintBookData(curr->data);

		curr = curr->next;
	}
}

void PrintAllReadBooks(struct ListNode *head)
{
	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->haveRead == 1)
			PrintBookData(curr->data);

		curr = curr->next;
	}
}

void PrintAllInterestedBooks(struct ListNode *head)
{
	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->isInterested == 1)
			PrintBookData(curr->data);

		curr = curr->next;
	}
}

void PrintAllBooksOfCategory(struct ListNode *head)
{
	DisplayCategories();
	enum Category category = SetCategory();

	char border[87] = {[0 ... 86] = '='};
	char line[87] = {[0 ... 86] = '-'};

	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);


	printf("| %-3s | %-30s | %-8s | %-5s | %-11s | %-11s |\n",
				"id", "name", "type", "Read", "Interested", "category");


	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, line);

	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->category == category)
			PrintBookData(curr->data);

		curr = curr->next;
	}

	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);
}

void PrintBookById(struct ListNode *head)
{
	unsigned int id = 0;

	printf("Enter book id\n");
	scanf("%u", &id);

	char border[87] = {[0 ... 86] = '='};
	char line[87] = {[0 ... 86] = '-'};

	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);


	printf("| %-3s | %-30s | %-8s | %-5s | %-11s | %-11s |\n",
				"id", "name", "type", "Read", "Interested", "category");


	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, line);

	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->id == id)
		{
			PrintBookData(curr->data);
			printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);

			return;
		}

		curr = curr->next;
	}
}

struct Book *GetBookById(unsigned int id, struct ListNode *head)
{
	struct ListNode *curr = head;
	while (curr)
	{
		if(curr->data->id == id)
			return curr->data;

		curr = curr->next;
	}

	return NULL;
}

void SetBookData(void (*setFunc)(struct Book *book), struct ListNode *head)
{
	unsigned int id;

	printf("enter book id\n");
	scanf("%d", &id);

	struct Book *book = GetBookById(id, head);
	if(!book)
	{
		printf(ANSI_COLOR_RED "Error: book is null\n" ANSI_COLOR_RESET);
		return;
	}

	setFunc(book);
}

void Add(struct Database *database, struct Book *book)
{
	DebugPrintf(ANSI_COLOR_MAGENTA "Add func book %s\n" ANSI_COLOR_RESET, book->name);

	static int id = 0;
	id++;
	book->id = id;

	struct ListNode *head = database->head;

	if (!head)
	{
		DebugPrintf(ANSI_COLOR_RED "List head is NULL\n" ANSI_COLOR_RESET);

		database->head = malloc(sizeof(struct ListNode));
		database->head->data = book;
		database->head->next = NULL;
		database->count++;

		DebugPrintf(ANSI_COLOR_GREEN "Added book to list\n" ANSI_COLOR_RESET);

		return;
	}

	while(head->next)
	{
		head = head->next;
	}

	DebugPrintf(ANSI_COLOR_ORANGE "Allocating book memory\n" ANSI_COLOR_RESET);

	head->next = malloc(sizeof(struct ListNode));
	head->next->data = book;
	head->next->next = NULL;
	database->count++;
}

void WriteDataToFile(struct Database *database, const char *path)
{
	DebugPrintf(ANSI_COLOR_ORANGE "Path: %s\n" ANSI_COLOR_RESET, path);
	DebugPrintf(ANSI_COLOR_ORANGE "Writing data to file\n" ANSI_COLOR_RESET);

	FILE *fp = fopen(path, "w+");
	DebugPrintf("Opened file\n");

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

void DeleteDatabaseData(struct Database *database)
{
	struct ListNode *curr = database->head;
	struct ListNode *prev = database->head;

	while(curr)
	{
		free(curr->data);
		curr = curr->next;

		free(prev);
		prev = curr;
	}
}

void PrintRecords(void (*func)(struct ListNode *), struct ListNode *head)
{
	char border[91] = {[0 ... 89] = '=', [90] = '\0'};
	char line[91] = {[0 ... 89] = '-', [90] = '\0'};

	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);


	printf("| %-3s | %-30s | %-8s | %-5s | %-11s | %-14s |\n",
				"id", "name", "type", "Read", "Interested", "category");


	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, line);

	func(head);


	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);
}

int CheckBook(struct Database *database, const char *name)
{
	DebugPrintf(ANSI_COLOR_ORANGE "Check book: %s\n" ANSI_COLOR_RESET, name);

	struct ListNode *head = database->head;

	if(!head)
	{
		DebugPrintf(ANSI_COLOR_RED "Head of list is null\n" ANSI_COLOR_RESET);
		return -1;
	}

	while(head)
	{
		if (strcmp(head->data->name, name) == 0)
		{
			DebugPrintf("Names match!\n%s : %s\n", head->data->name, name);
			return 0;
		}

		head = head->next;
	}

	return -1;
}
