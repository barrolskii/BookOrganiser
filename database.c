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

void PrintAllBooksOfExtension(const char *ext, struct ListNode *head)
{
	struct ListNode *curr = head;

	while(curr)
	{
		if(strcmp(ext, curr->data->type))
			PrintBookData(curr->data);
	}
}


void PrintAllUnreadBooks(struct ListNode *head)
{
	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->haveRead == 0)
			PrintBookData(curr->data);
	}
}

void PrintAllReadBooks(struct ListNode *head)
{
	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->haveRead == 1)
			PrintBookData(curr->data);
	}
}

void PrintAllInterestedBooks(struct ListNode *head)
{
	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->isInterested == 1)
			PrintBookData(curr->data);
	}
}

void PrintAllBooksOfCategory(enum Category category, struct ListNode *head)
{
	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->category == category)
			PrintBookData(curr->data);
	}
}

void PrintBookById(struct ListNode *head, unsigned int id)
{
	struct ListNode *curr = head;

	while(curr)
	{
		if(curr->data->id == id)
		{
			PrintBookData(curr->data);
			return;
		}
	}
}

void Add(struct Database *database, struct Book *book)
{
	struct ListNode *head = database->head;

	if (!head)
	{
		DebugPrintf(ANSI_COLOR_RED "List head is NULL\n" ANSI_COLOR_RESET);

		database->head = malloc(sizeof(struct ListNode));
		database->head->data = book;
		database->count++;

		return;
	}

	while(head->next)
	{
		head = head->next;
	}

	head->next = malloc(sizeof(struct ListNode));
	head->next->data = book;
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

void PrintRecords()
{
	printf(ANSI_COLOR_CYAN
			"---------------------------------------------------------------\n"
			ANSI_COLOR_RESET);

	// TODO: Implement function pointer print calls

	printf(ANSI_COLOR_CYAN
			"---------------------------------------------------------------\n"
			ANSI_COLOR_RESET);
}

int CheckBook(struct Database *database, const char *name)
{
	DebugPrintf("Check book: %s\n", name);

	struct ListNode *head = database->head;
	while(head)
	{
		if (strcmp(head->data->name, name) == 0)
		{
			DebugPrintf("Names match!\n%s : %s\n", head->data->name, name);
			return 0;
		}
	}

	return -1;
}
