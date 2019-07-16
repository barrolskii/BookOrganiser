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
	scanf("%s", ext);

	while(curr)
	{
		if(strcmp(ext, curr->data->type) == 0)
			PrintBookData(curr->data);

		curr = curr->next;
	}
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
	struct ListNode *curr = head;

	enum Category category = SetCategory();

	while(curr)
	{
		if(curr->data->category == category)
			PrintBookData(curr->data);

		curr = curr->next;
	}
}

void PrintBookById(struct ListNode *head)
{
	struct ListNode *curr = head;

	unsigned int id = 0;
	scanf("%u", &id);

	while(curr)
	{
		if(curr->data->id == id)
		{
			PrintBookData(curr->data);
			return;
		}

		curr = curr->next;
	}
}

void Add(struct Database *database, struct Book *book)
{
	printf(ANSI_COLOR_MAGENTA "Add func book %s\n" ANSI_COLOR_RESET, book->name);

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

		return;
	}

	while(head->next)
	{
		head = head->next;
	}

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
	char border[87] = {[0 ... 86] = '='};

	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);


	printf("| %-3s | %-30s | %-8s | %-5s | %-11s | %-11s |\n",
				"id", "name", "type", "Read", "Interested", "category");


	printf(ANSI_COLOR_CYAN
			"---------------------------------------------------------------\n"
			ANSI_COLOR_RESET);

	func(head);


	printf(ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, border);
}

int CheckBook(struct Database *database, const char *name)
{
	DebugPrintf(ANSI_COLOR_ORANGE "Check book: %s\n" ANSI_COLOR_RESET, name);

	struct ListNode *head = database->head;

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
