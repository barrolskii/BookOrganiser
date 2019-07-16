#ifndef BOOK_H
#define BOOK_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "terminalColors.h"
#include "utility.h"

#define MAX_BOOK_NAME_SIZE 128
#define MAX_EXTENSION_SIZE 8
#define TOTAL_CATEGORIES 6

enum Category
{
	MATHS = 0,
	SOFTWARE,
	HARDWARE,
	COMPUTING,
	DOCUMENTATION,
	OTHER,
};

static const char *categories[6] = {
	"Maths",
	"Software",
	"Hardware",
	"Computing",
	"Documentation",
	"Other"
};

enum Category SetCategory();
enum Category SetCategoryFromValue(unsigned int value);

void DisplayCategories();


struct Book
{
	unsigned int id;
	char name[MAX_BOOK_NAME_SIZE];
	char type[MAX_EXTENSION_SIZE];
	bool haveRead;		// Has the user previously read this book?
	bool isInterested;	// Is the user interested in reading this book?
	enum Category category;
};

void PrintBookData(struct Book *book);

void SetHaveRead(struct Book *book, bool state);
void SetIsInterested(struct Book *book, bool state);
void SetBookCategory(struct Book *book, enum Category category);

const char *GetBookCategory(struct Book *book);
char *ParseBookDataForWriting(struct Book *book);

#endif // BOOK_H
