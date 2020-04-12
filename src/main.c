#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <string.h>
#include <libkoios.h>
#include <stdarg.h>
#include <stdlib.h>

#include <readline/readline.h>

#include "utils.h"
#include "categories.h"
#include "terminalColors.h"


// Path to parent directory where all books are stored
char *parent_path = NULL;

// Division index. This is used for the division auto complete function
// so that we don't have dupliacte functions for each division
// There is probably a better way to pass the division but for now
// this works well
unsigned int div_index = 0;

char **tag_list = NULL; 
char **book_list = NULL;

koios_state state = {};
koios_mask mask = {};

// {{{
// From https://gitlab.com/finnoleary/koios/-/blob/master/koios.c 
// I can't take any credit for this function.
#define eprint(...) \
	fprintf(stderr, __VA_ARGS__)

#define mkpath(...) mkpath_(__VA_ARGS__, (char*)NULL)
static char *mkpath_(char *s, ...)
{
	char *s_orig = s;
	char *s_next = NULL;
	char *d = NULL;
	size_t length = 0;
	size_t offset = 0;
	va_list ap;

	if (!s) {
		eprint("Error creating path.\n");
		return NULL;
	}

	/* find length of path plus how many slashes to add */
	va_start(ap, s);
	while (s) {
		size_t size, slash;
		s_next = va_arg(ap, char *);
		size = strlen(s);
		slash = (s_next && s[size] != '/' && s_next[0] != '/');

		length += size+slash;
		s = s_next;
	}
	va_end(ap);

	s = s_orig;
	if (!(d = calloc(length+1, 1))) {
		perror("calloc");
		eprint("Error creating path\n");
		return NULL;
	}

	/* Join the paths together */
	va_start(ap, s);
	do {
		size_t size, slash;

		s_next = va_arg(ap, char *);
		size = strlen(s);
		/* NB: Possible bug? we do (size-1) not size here */
		slash = (s_next && s[size-1] != '/' && s_next[0] != '/');

		memcpy(d+offset, s, size);
		if (slash) { d[offset+size] = '/'; }
		offset += size+slash;
		s = s_next;
	} while (s);
	va_end(ap);
	d[offset] = '\0';

	return d;
}
// }}}


// {{{ Initalisation functions
void init(char *path)
{
	// Directory called books to contain, you guessed it, all the users books
	char books_dir[64] = {0};
	strcat(books_dir, path);
	strcat(books_dir, "books");

	printf("path: %s\n", books_dir);

	// Check if the book directory already exists
	// If function returns a negative then create the directory
	if (check_directory_exists(books_dir))
	{
		printf(ANSI_COLOR_RED "Books dir doesn't exist\n" ANSI_COLOR_RESET);
		create_directory(books_dir);
	}
	else
	{
		printf(ANSI_COLOR_GREEN "Books directory already exists\n" ANSI_COLOR_RESET);
	}
}

void move_files()
{
	DIR *dir = NULL;
	struct dirent *entry = NULL;

	char new_path[128] = {0};
	char old_path[128] = {0};

	koios_mask mask = {};


	if ((dir = opendir(parent_path)) != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			// Create a new mask for each file so tags don't stack from the
			// previous file
			koios_mask_new(&state, &mask);
			koios_tag tag;

			if (entry->d_type == DT_REG)
			{
				// Save the old path of the file
				strcat(old_path, parent_path);
				strcat(old_path, entry->d_name);

				// Make the new path ready to move the file
				strcat(new_path, parent_path);
				strcat(new_path, "books/");
				strcat(new_path, entry->d_name);


				printf("File: %s\n", entry->d_name);
				printf("Please select a class for the book\n");

				printf("new path: %s\n", new_path);
				printf("old path: %s\n", old_path);

				// print classes first
				print_border(print_str_array, main_classes, TOTAL_CLASSES);

				int class_input;
				printf("Select a book class\n");
				scanf("%d", &class_input);
				printf("class_input: %d\n", class_input);

				// Add the class tag to the file
				koios_name_find(&state, main_classes[class_input - 1], &tag);
				koios_tag_addtomask(&state, &mask, tag);

				// Then the divisions
				print_border(print_str_array, divisions[class_input - 1], TOTAL_DIVISIONS);

				int division_input;
				printf("Select a book division\n");
				scanf("%d", &division_input);
				printf("division_input: %d\n", division_input);

				// Add the division to the file
				koios_name_find(&state, divisions[class_input - 1][division_input - 1], &tag);
				koios_tag_addtomask(&state, &mask, tag);


				// Save the masks to the file
				koios_mask_save(&state, &mask, old_path);

				// Move the file to the books directory
				rename(old_path, new_path);

				// Clear the arrays when we are done with them
				memset(new_path, 0, sizeof(new_path));
				memset(old_path, 0, sizeof(old_path));

				// Delete the mask ready for a new file
				koios_mask_del(&mask);
			}
		}
	}

	closedir(dir);
}

int add_koios_tag_to_database(koios_state *state, char *tag_name, char *path)
{
	printf(ANSI_COLOR_GREEN "Adding tag: %s\n" ANSI_COLOR_RESET, tag_name);

	int value = koios_name_set(state, NULL, tag_name);
	printf("Add tag value: %d\n", value);

	if (!value) return -1;

	return 0;
}

void check_koios_tags(char *config_path)
{
	printf(ANSI_COLOR_GREEN "Checking tags\n" ANSI_COLOR_RESET);

	koios_tag tag;

	int i = 0;
	for (i; i < TOTAL_CLASSES; i++)
	{
		if (!koios_name_find(&state, main_classes[i], &tag))
		{
			add_koios_tag_to_database(&state, main_classes[i], config_path);
		}
		else
		{
			printf(ANSI_COLOR_RED "Tag: %s already exists\n" ANSI_COLOR_RESET, main_classes[i]);
		}
	}

	for (i = 0; i < 10; i++)
	{
		for (int j = 0; j < TOTAL_DIVISIONS; j++)
		{
			if (!koios_name_find(&state, divisions[i][j], &tag))
			{
				add_koios_tag_to_database(&state, divisions[i][j], config_path);
			}
			else
			{
				printf(ANSI_COLOR_RED "Tag: %s already exists\n" ANSI_COLOR_RESET, divisions[i][j]);
			}
		}
	}

	char *other_tags[2] = {
		"Have_Read",	// Have red and to reed
		"To_Read"		// This is when I dislike the English language...
	};

	for (i = 0; i < 2; i++)
	{
		if (!koios_name_find(&state, other_tags[i], &tag))
		{
			add_koios_tag_to_database(&state, other_tags[i], config_path);
		}
		else
		{
			printf(ANSI_COLOR_RED "Tag: %s already exists\n" ANSI_COLOR_RESET, other_tags[i]);
		}
	}

	printf(ANSI_COLOR_GREEN "Finished checking tags\n" ANSI_COLOR_RESET);

	// Save the changes to the database
	koios_cfg_store(&state, config_path);
}
// }}}


// Auto completion functions {{{

char *tag_name_generator(const char *text, int state)
{
	static int list_index, len;
	char *name = NULL;

	if (!state)
	{
		list_index = 0;
		len = strlen(text);
	}

	while((name = tag_list[list_index++]))
	{
		if (strncmp(name, text, len) == 0)
			return strdup(name);
	}

	return NULL;
}

char **tag_name_completion(const char *text, int start, int end)
{
	rl_attempted_completion_over = 1;
	return rl_completion_matches(text, tag_name_generator);
}

char *book_name_generator(const char *text, int state)
{
	static int list_index, len;
	char *name = NULL;

	if (!state)
	{
		list_index = 0;
		len = strlen(text);
	}

	while((name = book_list[list_index++]))
	{
		if (strncmp(name, text, len) == 0)
			return strdup(name);
	}

	return NULL;
}

char **book_name_completion(const char *text, int start, int end)
{
	rl_attempted_completion_over = 1;
	return rl_completion_matches(text, book_name_generator);
}

	/*
	 * TESTING CODE
	 */

/*	printf("%s\n", test[div_index][0]);

	rl_attempted_completion_function = division_name_completion;

	printf("Enter a division name\n");

	char *buffer = readline(">> ");

	int length = strlen(buffer);
	char *last_char = &buffer[length - 1];

	printf("last_char: %d\n", *last_char);

	if (*last_char == 32)
	{
		char new_word[length - 1];
		for (int i = 0; i < length -1; i++)
		{
			new_word[i] = buffer[i];
		}

		printf("new_word: %s:\n", new_word);
	}


	if (buffer)
	{
		printf("You entered: %s:\n", buffer);
		free(buffer);
	}


	return 0;*/


	/*
	 * END TESTING CODE
	 */


// }}}


// {{{ Template tests

// koios_tag *tag, koios_mask *mask, char *path, DIR *dir
void test_get_books(koios_tag *tag, koios_mask *mask, char *file_path, char *books_path, DIR *dir)
{
	char tag_name[128] = {0};
	struct dirent *entry = NULL;


	printf("Enter tag name you want to seach by\n");
	scanf("%s", tag_name);

	printf("tag_name: %s\n", tag_name);
	koios_name_find(&state, tag_name, tag);


	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_REG)
		{
			strcat(file_path, books_path);
			strcat(file_path, entry->d_name);

			// Create a fresh mask and load the current files tag mask in
			koios_mask_new(&state, mask);
			koios_mask_load(&state, mask, file_path);

			// Check if the file contains the tag mask and print it
			int contains = koios_tag_maskcontains(&state, mask, *tag);
			if (contains) printf("%s\n", entry->d_name);

			// Cleanup for the next iteration
			memset(file_path, 0, 128);
			koios_mask_del(mask);
		}
	}

}

void test_template(char *books_path, void (*fun)(koios_tag *, koios_mask *,  char *, char *, DIR *))
{
	koios_tag tag;
	koios_mask mask;

	char file_path[128] = {0};

	DIR *dir = NULL;


	if ((dir = opendir(books_path)) != NULL)
	{
		fun(&tag, &mask, file_path, books_path, dir);
	}


	closedir(dir);
}
// }}}


// {{{ Menu functions

void get_books_by_tag(char *books_path)
{
	koios_tag tag;

	char tag_name[128] = {0};
	char file_path[128] = {0};

	DIR *dir = NULL;
	struct dirent *entry = NULL;

	printf("Enter tag name you want to seach by\n");
	scanf("%s", tag_name);

	printf("tag_name: %s\n", tag_name);
	koios_name_find(&state, tag_name, &tag);


	if ((dir = opendir(books_path)) != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			if (entry->d_type == DT_REG)
			{
				strcat(file_path, books_path);
				strcat(file_path, entry->d_name);

				// Load the current file mask
				koios_mask_load(&state, &mask, file_path);

				// Check if the file contains the tag mask and print it
				int contains = koios_tag_maskcontains(&state, &mask, tag);
				if (contains) printf("%s\n", entry->d_name);

				// Cleanup for the next iteration
				memset(file_path, 0, 128);
			}
		}
	}

	closedir(dir);
}

void get_books_by_class(char *books_path)
{	
	koios_tag tag;

	char file_path[128] = {0};

	DIR *dir = NULL;
	struct dirent *entry = NULL;

	int input = 0;

	// Show the user the selection of classes
	printf("Select a class\n");
	print_border(print_str_array, main_classes, TOTAL_CLASSES);


	scanf("%d", &input);
	printf("input: %d\n", input);

	printf("Chosen class: %s\n", main_classes[input - 1]);
	koios_name_find(&state, main_classes[input - 1], &tag);

	if ((dir = opendir(books_path)) != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			if (entry->d_type == DT_REG)
			{
				// Concatenate the current file to the end of the books directory path
				// to get the full file path
				strcat(file_path, books_path);
				strcat(file_path, entry->d_name);

				printf("file path: %s\n", file_path);

				// Load the current files mask
				int ml = koios_mask_load(&state, &mask, file_path);
				printf("mask load: %d\n", ml);
				printf("Error: %s\n", koios_errstr(ml));

				// Check if the file contains the tag mask and print it
				int contains = koios_tag_maskcontains(&state, &mask, tag);
				if (contains) printf("%s\n", entry->d_name);

				memset(file_path, 0, 128);
			}
		}
	}

	closedir(dir);
}

void add_tag_to_book(char *books_path)
{
	koios_tag tag;

	char file_name[128] = {0};
	char file_path[128] = {0};
	char new_tag[128]	= {0};

	printf("Enter book you want to add a new tag to\n");
	scanf("%s", file_name);

	printf("Enter the tag you want to add\n");
	scanf("%s", new_tag);

	strcat(file_path, books_path);
	strcat(file_path, file_name);

	int exists = koios_name_find(&state, new_tag, &tag);
	printf("exists %d\n", exists);
	if (!exists)
	{
		printf("Tag does not exist\n");
		return;
	}

	koios_mask_load(&state, &mask, file_path);

	int contains = koios_tag_maskcontains(&state, &mask, tag);
	if (!contains) koios_tag_addtomask(&state, &mask, tag);

	koios_mask_save(&state, &mask, file_path);
}

void show_books_to_read(char *books_path)
{
	koios_tag tag;

	char file_path[128] = {0};

	DIR *dir = NULL;
	struct dirent *entry = NULL;

	char *tag_name = "To_Read";


	// Search for the "To_Read" tag in the koios database to load it into the 
	// tag struct
	koios_name_find(&state, tag_name, &tag);

	if ((dir = opendir(books_path)) != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			if (entry->d_type == DT_REG)
			{
				strcat(file_path, books_path);
				strcat(file_path, entry->d_name);

				koios_mask_load(&state, &mask, file_path);

				// Check if the file contains the tag mask and print it
				int contains = koios_tag_maskcontains(&state, &mask, tag);
				if (contains) printf("%s\n", entry->d_name);

				// Cleanup for the next iteration
				memset(file_path, 0, 128);
			}
		}
	}

	closedir(dir);
}

void set_books_to_read(char *books_path)
{
	koios_tag tag;

	char file_name[128] = {0};
	char file_path[128] = {0};

	char *tag_name = "To_Read";

	printf("Enter book you want to set to read\n");
	scanf("%s", file_name);

	strcat(file_path, books_path);
	strcat(file_path, file_name);


	koios_name_find(&state, tag_name, &tag);

	koios_mask_load(&state, &mask, file_path);

	int contains = koios_tag_maskcontains(&state, &mask, tag);
	if (!contains) koios_tag_addtomask(&state, &mask, tag);

	koios_mask_save(&state, &mask, file_path);
}

// }}}


// Config functions {{{

unsigned int get_koios_tag_count(char *path)
{
	char str[256];
	unsigned int tag_count = 0;

	FILE *fp = fopen(path, "r");

	if(!fp)
	{
		printf(ANSI_COLOR_RED "unable to find file\n" ANSI_COLOR_RESET);
		return tag_count;
	}

	while(fgets(str, 256, fp) != NULL)
	{
		// The config file is populated with lots of lines that
		// are just a single newline character. So once we hit
		// the first line containing only the newline character
		// then we are done counting the tags
		if (strlen(str) < 2) break;

		tag_count++;
	}

	fclose(fp);

	// Decrement the tag count as the first line of the config 
	// file is not a tag
	tag_count--;

	return tag_count;
}

void populate_tag_list(char **list, int size, char *path)
{
	char str[256];
	int length = 0;

	FILE *fp = fopen(path, "r");

	if(!fp)
	{
		printf(ANSI_COLOR_RED "unable to find file\n" ANSI_COLOR_RESET);
		return;
	}

	// Skip the first line in the file
	fgets(str, 256, fp);

	for (int i = 0; i < size; i++)
	{
		fgets(str, 256, fp);


		length = strlen(str);

		if (str[length - 1] == 10) str[length - 1] = '\0';

		list[i] = malloc(sizeof(char) * length);
		strcpy(list[i], str);
		memset(str, 0, 256);
	}	

	list[size] = NULL;

	fclose(fp);
}

void populate_book_list(char **list, int size, char *path)
{
	DIR *dir = NULL;
	struct dirent *entry = NULL;

	int i = 0;


	if ((dir = opendir(path)) != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			if (entry->d_type == DT_REG)
			{
				int length = strlen(entry->d_name);

				list[i] = malloc((sizeof(char) * length) + 1);
				strcpy(list[i], entry->d_name);

				i++;
			}
		}
	}

	list[i] = NULL;
}

void get_tag()
{
	rl_attempted_completion_function = tag_name_completion;

	printf("Enter a tag\n");

	char *buffer = readline(">> ");

	if (buffer)
	{
		printf("You entered: %s:\n", buffer);
		free(buffer);
	}
}

void get_book()
{
	rl_attempted_completion_function = book_name_completion;

	printf("Enter a book\n");

	char *buffer = readline(">> ");

	if (buffer)
	{
		printf("You entered: %s:\n", buffer);
		free(buffer);
	}
}

// }}}


int main(int argc, char **argv)
{
	if (argc == 1)
	{
		// If no path is given assume directory above the source folder is 
		// the main parent directory for all books
		parent_path = "../";
	}
	else
	{
		parent_path = argv[1];
	}

	char *config_path = mkpath(getenv("HOME"), ".config", ".koios.cfg");

	// Initalise the state and the mask
	koios_cfg_open(&state, config_path);

	int mn = koios_mask_new(&state, &mask);
	printf("mask new %d\n", mn);
	printf("Error: %s\n", koios_errstr(mn));

	init(parent_path);
	check_koios_tags(config_path);

	unsigned int file_count = count_files_in_directory(parent_path);
	if (file_count > 0)
	{
		move_files();
	}
	else
	{
		printf("No new files have been found\n");
	}

	char books_path[128] = {0};
	strcat(books_path, parent_path);
	strcat(books_path, "books/");


	unsigned int tag_count = get_koios_tag_count(config_path);

	// Increment tag count so that the tag list will have NULL
	// as its very last element for the autocompletion
	tag_count++;

	printf("tag_count: %d\n", tag_count);

	// Allocate enough memory for the list of tags
	tag_list = malloc(sizeof(char*) * tag_count);
	populate_tag_list(tag_list, tag_count, config_path);


	unsigned int book_count = count_files_in_directory(books_path);
	printf("book_count: %d\n", book_count);

	// Same as the tag count, increment value for the last NULL element
	book_count++;

	book_list = malloc(sizeof(char*) * book_count);
	populate_book_list(book_list, book_count, books_path);


	get_tag();
	get_book();

	// Main loop
	char input;
	int int_input;

	while (1)
	{
		print_border(print_str_array, menu_options, MENU_OPTIONS_SIZE);


		printf("input value\n");
		scanf(" %c", &input); // "<space>%c" allows us to ignore any trailing
							  // whitespace from previous input


		if (input == 'q') break;

		int_input = atoi(&input);
		switch (int_input)
		{
			case 1:
				printf("Get books by tag called\n");

				//test_template(books_path, test_get_books);
				get_books_by_tag(books_path);
				break;
			case 2:
				printf("Get books by class\n");
				get_books_by_class(books_path);
				break;
			case 3:
				printf("Add tag to book\n");
				add_tag_to_book(books_path);
				break;
			case 4:
				printf("Show books to read\n");
				show_books_to_read(books_path);
				break;
			case 5:
				printf("Set books to read\n");
				set_books_to_read(books_path);
				break;
			default:
				printf("Please enter a valid number\n");
		}

	}

	int i = 0;

	for (i; i < tag_count; i++)
	{
		free(tag_list[i]);
	}
	free(tag_list);

	for (i = 0; i < book_count; i++)
	{
		free(book_list[i]);
	}
	free(book_list);

	// Clean up the koios state when we're done
	koios_cfg_close(&state);

	free(config_path);

	int md = koios_mask_del(&mask);
	printf("mask del: %d\n", md);


	return 0;
}

