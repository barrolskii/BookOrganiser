#ifndef DATABASE_H
#define DATABASE_H

#include "stddef.h"
#include "stdlib.h"
#include "stdio.h"

#include "terminalColors.h"
#include "utility.h"
#include "book.h"

struct ListNode
{
	struct ListNode *next;
	struct Book *data;
};

void PrintAllRecords(struct ListNode *head);
void PrintAllBooksOfExtension(struct ListNode *head);

void PrintAllUnreadBooks(struct ListNode *head);
void PrintAllReadBooks(struct ListNode *head);

void PrintAllInterestedBooks(struct ListNode *head);
void PrintAllBooksOfCategory(struct ListNode *head);

void PrintBookById(struct ListNode *head);

struct Database
{
	struct ListNode *head;
	unsigned int count;
};

void Add(struct Database *database, struct Book *book);

void WriteDataToFile(struct Database *database, const char *path);
void DeleteDatabaseData(struct Database *database);

void PrintRecords();

int CheckBook(struct Database *database, const char *name);

#endif // DATABASE_H
