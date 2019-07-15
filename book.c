#include "book.h"

enum Category SetCategory()
{
	enum Category category;
	unsigned int input;

	printf("Set book category\n");
	scanf("%d", &input);


	switch(input)              
	{
		case 0:
			category = MATHS;
			break;
		case 1:
			category = SOFTWARE;
			break;
		case 2:   
			category = HARDWARE;
			break;
		case 3:
			category = COMPUTING;        
			break;
		case 4:
			category = DOCUMENTATION;    
			break;     
		case 5:                          
			category = OTHER;
			break;

		default:
			printf(ANSI_COLOR_RED
					"Value too large defaulting to other\n"
				   ANSI_COLOR_RESET);
			category = OTHER;
			break;
	}

	return category;
}

enum Category SetCategoryFromValue(unsigned int value)
{
	enum Category category;

	switch(value)              
	{
		case 0:
			category = MATHS;
			break;
		case 1:
			category = SOFTWARE;
			break;
		case 2:   
			category = HARDWARE;
			break;
		case 3:
			category = COMPUTING;        
			break;
		case 4:
			category = DOCUMENTATION;    
			break;     
		case 5:                          
			category = OTHER;
			break;

		default:
			printf(ANSI_COLOR_RED
					"Value too large defaulting to other\n"
				   ANSI_COLOR_RESET);
			category = OTHER;
			break;
	}

	return category;
}

void DisplayCategories()
{
	printf("=======================================================\n");

	for(int i = 0; i < 6; i++)
	{
		printf("|%s", categories[i]);
	}

	printf("\n=======================================================\n");
}

void PrintBookData(struct Book *book)
{
	printf("%s | %s\n", book->name, book->type);
}

void SetHaveRead(struct Book *book, bool state)
{
	book->haveRead = state;
}

void SetIsInterested(struct Book *book, bool state)
{
	book->isInterested = state;
}

void SetBookCategory(struct Book *book, enum Category category)
{
	book->category = category;
}

const char *GetBookCategory(struct Book *book)
{
	return categories[book->category];
}

char *ParseBookDataForWriting(struct Book *book)
{
	// 148 is the size of the book struct plus pipes for file and newline
	char *parsedData = malloc(148);

	strncpy(parsedData, book->name, 148);
	strcat(parsedData, "|");

	strcat(parsedData, book->type);
	strcat(parsedData, "|");

	strcat(parsedData, (book->haveRead != 0 ? "1" : "0"));
	strcat(parsedData, "|");

	strcat(parsedData, (book->isInterested != 0 ? "1" : "0"));
	strcat(parsedData, "|");

	const char *category = " ";
	switch(book->category)
	{
		case 0:
			category = "0";
			break;
		case 1:
			category = "1";
			break;
		case 2:
			category = "2";
			break;
		case 3:
			category = "3";
			break;
		case 4:
			category = "4";
			break;
		case 5:
			category = "5";
			break;
	}

	strcat(parsedData, category);

	DebugPrintf(ANSI_COLOR_RED "parsedData: %s\n" ANSI_COLOR_RESET, parsedData);
	strcat(parsedData, "\n\0");

	return parsedData;
}

