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
	char border[22] = {[0 ... 21] = '='};


	printf("%s\n", border);

	for(int i = 0; i < TOTAL_CATEGORIES; i++)
	{
		printf("| %-3d| %-14s|\n", i, categories[i]);
	}

	printf("%s\n", border);
}

void PrintBookData(struct Book *book)
{
	printf("| %-3u | %-30s | %-8s | %-5d | %-11d | %-14s |\n", 
							book->id, book->name, book->type, book->haveRead,
							book->isInterested, GetBookCategory(book));
}

void SetHaveRead(struct Book *book)
{
	book->haveRead = !book->haveRead;

	printf(ANSI_COLOR_GREEN
			"The book %s\n have read property has been set to %d from %d\n"
		   ANSI_COLOR_RESET, book->name, book->haveRead, !book->haveRead);
}

void SetIsInterested(struct Book *book)
{
	book->isInterested = !book->isInterested;

	printf(ANSI_COLOR_GREEN
			"The book %s\n is interested property has been set to %d from %d\n"
		   ANSI_COLOR_RESET, 
		   book->name, book->isInterested, !book->isInterested);
}

void SetBookCategory(struct Book *book)
{
	enum Category prevCat = book->category;

	book->category = SetCategory();

	printf(ANSI_COLOR_GREEN
			"The book %s\n has had its category changed from %s to %s\n"
		   ANSI_COLOR_RESET,
			book->name, categories[prevCat], categories[book->category]);
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

