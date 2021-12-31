#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>

#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include <sys/stat.h>

#define ENTER 10
#define ESCAPE_KEY 27
#define MAX_LENGTH NAME_MAX + PATH_MAX

#define BOOK_DATA_FILE "books.csv"

#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

#define FREE_2D_ARR(arr, size)     \
    for (int i = 0; i < size; i++) \
        free(arr[i]);              \
    free(arr);                     \

typedef struct {
    char *name;
    char *tags;
    unsigned char have_read : 1;
    unsigned char in_prog : 1;
    unsigned char to_read : 1;
} book_t;

struct _book_node {
    book_t *book;
    struct _book_node *next;
};

typedef struct _book_node book_node_t;

char books_path[PATH_MAX] = {0};
char lib_path[PATH_MAX]   = {0};

unsigned book_count = 0;
book_t **book_data = NULL; /* Stores info about every book */

unsigned node_count = 0;
book_node_t *head  = NULL;

unsigned results_count = 0;
/* Item array containing search results */
ITEM **results = NULL;

/* Flag for whether or not we should update the book records */
/* This will only be true when new books have been added or  */
/* records have been updated */
int should_update = 0;

void clear_previous_results(void)
{
    /* Cleanup the previous results */
    if (results != NULL)
    {
        for (int i = 0; i < results_count; i++)
        {
            free((char*)results[i]->name.str);
            free_item(results[i]);
        }
        free(results);
        results_count = 0;
        results       = NULL;
    }
}

void print_main_menu_help_string(WINDOW *win)
{
    mvwprintw(win, 17, 1, "<J><DOWN> - Move Down      <K><UP> - Move Up");
    mvwprintw(win, 18, 1, "<ENTER>   - Select Option  <ESC> - Quit");
    wrefresh(win);
}

void print_list_books_help_string(WINDOW *win)
{
    mvwprintw(win, 17, 1, "<R> - Set To Read    <P>   - Set In Progress ");
    mvwprintw(win, 18, 1, "<H> - Set Have Read  <ESC> - Quit");
    wrefresh(win);
}

book_t *init_book(const char *name, const char *tags)
{
    book_t *new_book = calloc(1, sizeof(book_t));

    new_book->name = malloc( (sizeof(char) * strlen(name)) + 1);
    strcpy(new_book->name, name);
    new_book->name[strlen(new_book->name)] = '\0';

    new_book->tags = malloc( (sizeof(char) * strlen(tags)) + 1);
    strcpy(new_book->tags, tags);
    new_book->tags[strlen(new_book->tags)] = '\0';

    /* This fixes an issue with the form character padding       */
    /* I've tried changing it to be a NULL termination character */
    /* but that doesn't work. So we just find the last valid     */
    /* character and put a NULL terminator after that            */
    for (int i = strlen(new_book->tags); i > 0; i--)
    {
        if ((new_book->tags[i] >= 33) && (new_book->tags[i] <= 126))
        {
            new_book->tags[i + 1] = '\0';
            break;
        }
    }

    return new_book;
}

int get_book_index(const char *name)
{
    for (int i = 0; i < book_count; i++)
    {
        if (*name != book_data[i]->name[0])
            continue;

        if (strcmp(name, book_data[i]->name) == 0)
            return i;
    }

    return -1;
}

void print_book_info(WINDOW *win, book_t *book)
{
        werase(win);

        // TODO: Split name on multiple lines if it is too long
        mvwprintw(win, 1, 1, "%s",             book->name);
        mvwprintw(win, 5, 1, "%s",             book->tags);
        mvwprintw(win, 10, 1, "To read: %d",   book->to_read);
        mvwprintw(win, 11, 1, "In prog: %d",   book->in_prog);
        mvwprintw(win, 12, 1, "Have read: %d", book->have_read);

        box(win, 0, 0);
        wrefresh(win);

}

void print_book_info_from_name(WINDOW *win, const char *book_name)
{
    int index = get_book_index(book_name);

    if (index == -1)
    {
        mvwprintw(win, 1, 1, "Error: Cannot find book [%s]", book_name);
    }
    else
    {
        /* Clear any old output */
        werase(win);

        book_t *book = book_data[index];

        // TODO: Split name on multiple lines if it is too long
        mvwprintw(win, 1, 1, "%s",             book->name);
        mvwprintw(win, 5, 1, "%s",             book->tags);
        mvwprintw(win, 10, 1, "To read: %d",   book->to_read);
        mvwprintw(win, 11, 1, "In prog: %d",   book->in_prog);
        mvwprintw(win, 12, 1, "Have read: %d", book->have_read);

        box(win, 0, 0);
        wrefresh(win);
    }

}

void free_book(book_t *book)
{
    free(book->name);
    free(book->tags);
    free(book);
}

book_node_t *init_book_node(const char *name, const char *tags)
{
    book_node_t *new_node = calloc(1, sizeof(book_t));
    new_node->book = init_book(name, tags);

    return new_node;
}

int add_node(book_node_t *node)
{
    if (!head)
    {
        head = node;
        ++node_count;
        should_update = 1;
        return 1;
    }

    book_node_t *curr = head;

    while (curr->next)
    {
        curr = curr->next;
    }

    curr->next = node;
    ++node_count;
    should_update = 1;

    return 1;
}

book_t *node_contains(const char *name)
{
    book_node_t *node = head;

    while (node)
    {
        if (strcmp(node->book->name, name) == 0)
            return node->book;

        node = node->next;
    }

    return NULL;
}

void free_node(book_node_t *node)
{
    free_book(node->book);
    free(node);
}

// TODO: Helper file
int dir_exists(char *path)
{
    DIR *dir = NULL;

    if ((dir = opendir(path)) != NULL)
    {
        closedir(dir);
        return 1;
    }

    closedir(dir);
    return 0;
}

unsigned dir_count_files(char *path)
{
    unsigned count = 0;

    DIR *dir = NULL;
    struct dirent *entry = NULL;

    if ((dir = opendir(path)) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_REG)
                ++count;
        }
    }

    closedir(dir);

    return count;
}

char **dir_get_files(char *path)
{
    int i = 0;
    unsigned file_count = dir_count_files(path);

    char **files = calloc(file_count, sizeof(char*));
    for (int i = 0; i < file_count; i++)
    {
        files[i] = calloc(NAME_MAX, sizeof(char));
    }

    DIR *dir = NULL;
    struct dirent *entry = NULL;

    if ((dir = opendir(path)) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type != DT_REG)
                continue;

            memcpy(files[i], entry->d_name, NAME_MAX);
            i++;
        }
    }

    closedir(dir);

    return files;
}

void init(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Error: Path must be supplied\n");
        exit(1);
    }

    /* We're subtracting 10 from the max path because when we copy the string      */
    /* over to the library path the last directory will be /library/               */
    /* /library/ does only have a length of 9 but we're including the null         */
    /* termination character                                                       */
    /* eg. ~/Documents/Dir/MyBooks/library/                                        */
    /* The filesystem on my machine is EXT4 which doens't actually have any limits */
    /*  to path length but we do have to put a limit in                            */
    /* somewhere to allocate enough memory.                                        */
    if (strlen(argv[1]) > (PATH_MAX - 10))
    {
        fprintf(stderr, "Error: Provided path length is greater than maximum\n");
        fprintf(stderr, "       supported path. Please supply a path that has\n");
        fprintf(stderr, "       a length less than or equal to %d\n", PATH_MAX);
        exit(1);
    }

    strcpy(books_path, argv[1]);

    if (!dir_exists(books_path))
    {
        fprintf(stderr, "Error: [%s] does not exist\n", books_path);
        exit(1);
    }

    /* Check if a trailing '/' was supplied with the path */
    if (books_path[strlen(books_path) - 1] != '/')
    {
        books_path[strlen(books_path)] = '/';
    }

    strcpy(lib_path, books_path);

    if (lib_path[strlen(lib_path) - 1] == '/')
    {
        strcat(lib_path, "library/");
    }
    else
    {
        strcat(lib_path, "/library/");
    }

    /* Check if the library path exists and if not then create it */
    if (!dir_exists(lib_path))
    {
        if (mkdir(lib_path, S_IRWXU) != 0)
        {
            fprintf(stderr, "Error: Failed to create library path [%s].\n", lib_path);
            fprintf(stderr, "mkdir failed with error message [%s]\n", strerror(errno));
            exit(1);
        }
    }

    /* Check if data file for books exists */
    if (access(BOOK_DATA_FILE, E_OK) == 0)
    {
        FILE *fp = fopen(BOOK_DATA_FILE, "r");

        char name[1024] = {0};
        char tags[1024] = {0};
        char have_read;
        char in_prog;
        char to_read;

        fscanf(fp, "%d\n", &book_count);


        book_data = calloc(book_count, sizeof(book_t*));
        for (int i = 0; i < book_count; i++)
        {
            fscanf(fp, "%[^,],%c,%c,%c,%[^\n]\n", name, &have_read, &in_prog, &to_read, tags);

            book_data[i] = init_book(name, tags);
            book_data[i]->have_read = have_read;
            book_data[i]->in_prog   = in_prog;
            book_data[i]->to_read   = to_read;
        }

        fclose(fp);
    }
    else
    {
        FILE *file = fopen(BOOK_DATA_FILE, "w");
        fclose(file);
    }
}

void check_for_books(WINDOW *win, FORM *form, char *path)
{
    unsigned count = dir_count_files(path);

    werase(win);
    box(win, 0, 0);
    wrefresh(win);


    if (count == 0)
    {
        mvwprintw(win, 2, 2, "No new books detected");
        print_main_menu_help_string(win);
        wrefresh(win);
        return;
    }

    char **new_books = dir_get_files(path);

    post_form(form);
    wrefresh(win);

    char old_path[MAX_LENGTH] = {0};
    char new_path[MAX_LENGTH] = {0};

    int ch;

    for (int i = 0; i < count; i++)
    {
        /* Clear previous output and update the window with new output */
        wmove(win, 3, 1);
        wclrtoeol(win);
        box(win, 0, 0);
        waddnstr(win, new_books[i], 64);

        pos_form_cursor(form);

        while ((ch = wgetch(win)))
        {
            if (ch == ENTER)
            {
                form_driver(form, REQ_VALIDATION);

                book_node_t *curr_book = init_book_node(new_books[i], field_buffer(form->field[0], 0));
                add_node(curr_book);

                /* Copy the files path to old path */
                strcpy(old_path, path);
                strcat(old_path, new_books[i]);

                /* Now make the library path where the book will be moved to */
                strcpy(new_path, lib_path);
                strcat(new_path, new_books[i]);

                rename(old_path, new_path);

                memset(old_path, 0, MAX_LENGTH);
                memset(new_path, 0, MAX_LENGTH);

                form_driver(form, REQ_CLR_FIELD);

                break;
            }

            switch (ch)
            {
                case KEY_LEFT:
                    form_driver(form, REQ_PREV_CHAR);
                    break;
                case KEY_RIGHT:
                    form_driver(form, REQ_NEXT_CHAR);
                    break;
                case KEY_BACKSPACE:
                    form_driver(form, REQ_DEL_PREV);
                    break;
                case KEY_DC:
                    form_driver(form, REQ_DEL_CHAR);
                    break;
                case KEY_UP:
                    form_driver(form, REQ_END_LINE);
                    break;
                case ESCAPE_KEY:
                    goto book_cleanup;
                    break;
                default:
                    form_driver(form, ch);
            }

        }
    }

book_cleanup:
    form_driver(form, REQ_CLR_FIELD);
    unpost_form(form);
    werase(win);
    box(win, 0, 0);
    wrefresh(win);


    FREE_2D_ARR(new_books, count)
}

void search_for_book(WINDOW *menu_win, WINDOW *output_win, MENU *menu, FORM *form)
{
    clear_previous_results();

    werase(output_win);
    box(output_win, 0, 0);
    post_form(form);
    wrefresh(output_win);

    int ch;
    char search_opt[1024] = {0};

    pos_form_cursor(form);


    while ((ch = wgetch(output_win)))
    {
        if (ch == ENTER)
        {
            form_driver(form, REQ_VALIDATION);
            strcpy(search_opt, field_buffer(form->field[0], 0));

            form_driver(form, REQ_CLR_FIELD);
            unpost_form(form);

            // TODO: Function this to trim whitespace
            for (int i = strlen(search_opt); i > 0; i--)
            {
                if ((search_opt[i] >= 33) && (search_opt[i] <= 126))
                {
                    search_opt[i + 1] = '\0';
                    break;
                }
            }

            mvwprintw(output_win, 7, 1, "tags: %s", search_opt);
            wrefresh(output_win);

            break;
        }

        switch (ch)
        {
            case KEY_LEFT:
                form_driver(form, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(form, REQ_NEXT_CHAR);
                break;
            case KEY_BACKSPACE:
                form_driver(form, REQ_DEL_PREV);
                break;
            case KEY_DC:
                form_driver(form, REQ_DEL_CHAR);
                break;
            case KEY_UP:
                form_driver(form, REQ_END_LINE);
                break;
            case ESCAPE_KEY:
                goto clean_search;
                break;
            default:
                form_driver(form, ch);
        }
    }

    for (int i = 0; i < book_count; i++)
    {
        if (strstr(book_data[i]->tags, search_opt))
            ++results_count;
    }

    for (book_node_t *node = head; node; node = node->next)
    {
        if (strstr(node->book->tags, search_opt))
            ++results_count;
    }


    if (results_count == 0)
    {
        werase(output_win);
        box(output_win, 0, 0);
        mvwprintw(output_win, 2, 2, "No books found with tag/s");
        wrefresh(output_win);
    }
    else
    {
        char *name = NULL;
        results = calloc(results_count + 1, sizeof(ITEM*));

        int i = 0, j = 0;
        for (; i < book_count; i++)
        {
            if (strstr(book_data[i]->tags, search_opt))
            {
                int len = strlen(book_data[i]->name);
                name = calloc(len + 1, sizeof(char));
                strcpy(name, book_data[i]->name);

                results[j] = new_item(name, "");
                ++j;
            }
        }


        for (book_node_t *node = head; node; node = node->next)
        {
            if (strstr(node->book->tags, search_opt))
            {
                int len = strlen(node->book->name);
                name = calloc(len + 1, sizeof(char));
                strcpy(name, node->book->name);

                results[j] = new_item(name, "");
                ++j;
            }
        }

        name = NULL;

        int index;
        ITEM *curr_item = NULL;
        book_t *curr_book = NULL;

        set_menu_items(menu, results);
        post_menu(menu);
        pos_menu_cursor(menu);

        curr_item = current_item(menu);
        print_book_info_from_name(output_win, item_name(curr_item));

        while ((ch = wgetch(menu_win)) != ESCAPE_KEY)
        {
            switch (ch)
            {
                case 'j':
                case KEY_DOWN:
                    menu_driver(menu, REQ_DOWN_ITEM);
                    curr_item = current_item(menu);
                    index = get_book_index(item_name(curr_item));

                    if (index == -1)
                        curr_book = node_contains(item_name(curr_item));
                    else
                        curr_book = book_data[index];

                    break;
                case 'k':
                case KEY_UP:
                    menu_driver(menu, REQ_UP_ITEM);
                    curr_item = current_item(menu);
                    index = get_book_index(item_name(curr_item));

                    if (index == -1)
                        curr_book = node_contains(item_name(curr_item));
                    else
                        curr_book = book_data[index];
                    break;
            }



            if (curr_book)
                print_book_info(output_win, curr_book);
            else
                mvwprintw(output_win, 1, 1, "Error: Cannot find book [%s]", item_name(curr_item));
        }
    }

clean_search:
    unpost_menu(menu);
    form_driver(form, REQ_CLR_FIELD);
    unpost_form(form);
    wrefresh(output_win);
}

void list_book_to_read(WINDOW *menu_win, WINDOW *output_win, MENU *menu)
{
    clear_previous_results();

    for (int i = 0; i < book_count; i++)
    {
        if (book_data[i]->to_read)
            ++results_count;
    }


    for (book_node_t *node = head; node; node = node->next)
    {
        if (node->book->to_read)
            ++results_count;
    }

    if (results_count == 0)
    {
        werase(output_win);
        box(output_win, 0, 0);
        mvwprintw(output_win, 2, 2, "No books to read detected");
        print_main_menu_help_string(output_win);
        return;
    }
    else
    {
        char *name = NULL;
        results = calloc(results_count + 1, sizeof(ITEM*));

        int i = 0, j = 0;
        for (; i < book_count; i++)
        {
            if (book_data[i]->to_read)
            {
                int len = strlen(book_data[i]->name);
                name = calloc(len + 1, sizeof(char));
                strcpy(name, book_data[i]->name);

                results[j] = new_item(name, "");
                ++j;

                // TODO: Look into this. This could work it just needs some tweaking
                //results[j] = new_item(book_data[i]->name, "");
                //++j;
            }
        }

        for (book_node_t *node = head; node; node = node->next)
        {
            if (node->book->to_read)
            {
                int len = strlen(node->book->name);
                name = calloc(len + 1, sizeof(char));
                strcpy(name, node->book->name);

                results[j] = new_item(name, "");
                ++j;
            }
        }

        name = NULL;
    }

    int ch, index;
    ITEM *curr_item = NULL;
    book_t *curr_book;

    set_menu_items(menu, results);
    post_menu(menu);
    pos_menu_cursor(menu);

    curr_item = current_item(menu);
    print_book_info_from_name(output_win, item_name(curr_item));

    while ((ch = wgetch(menu_win)) != ESCAPE_KEY)
    {
        switch (ch)
        {
            case 'j':
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                curr_item = current_item(menu);
                index = get_book_index(item_name(curr_item));

                if (index == -1)
                    curr_book = node_contains(item_name(curr_item));
                else
                    curr_book = book_data[index];

                break;
            case 'k':
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                curr_item = current_item(menu);
                index = get_book_index(item_name(curr_item));

                if (index == -1)
                    curr_book = node_contains(item_name(curr_item));
                else
                    curr_book = book_data[index];

                break;
        }

        if (curr_book)
            print_book_info(output_win, curr_book);
        else
            mvwprintw(output_win, 1, 1, "Error: Cannot find book [%s]", item_name(curr_item));
    }

    unpost_menu(menu);
}

void list_books(WINDOW *menu_win, WINDOW *output_win, MENU *menu, char *path)
{
    clear_previous_results();

    results_count = dir_count_files(path);

    /* (file count + node count) + 1 as ncurses likes the last item to be NULL */
    results = calloc( (results_count + node_count) + 1, sizeof(ITEM*));

    int i      = 0;
    char *name = NULL;

    /* We read the files in the library directory so we can flag any files */
    /* that have been moved there by the user or other programs            */
    DIR *dir             = NULL;
    struct dirent *entry = NULL;

    if ((dir = opendir(path)) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type != DT_REG)
                continue;

            /* Heap allocating the name as ncurses items only store the pointer */
            /* to the name and description. We have to allocate and free name */
            /* memory ourselves */
            int len = strlen(entry->d_name);
            name = calloc(len + 1, sizeof(char));
            strcpy(name, entry->d_name);

            results[i] = new_item(name, "");

            i++;
        }
    }

    name = NULL;
    closedir(dir);

    int ch;
    ITEM *curr_item = NULL;

    set_menu_items(menu, results);
    post_menu(menu);
    pos_menu_cursor(menu);

    curr_item = current_item(menu);

    int index         = get_book_index(item_name(curr_item));
    book_t *curr_book = NULL;

    if (index == -1)
        curr_book = node_contains(item_name(curr_item));
    else
        curr_book = book_data[index];

    print_book_info(output_win, curr_book);
    print_list_books_help_string(output_win);

    while ((ch = wgetch(menu_win)) != ESCAPE_KEY)
    {
        switch (ch)
        {
            case 'j':
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                curr_item = current_item(menu);
                index = get_book_index(item_name(curr_item));

                if (index == -1)
                    curr_book = node_contains(item_name(curr_item));
                else
                    curr_book = book_data[index];
                break;
            case 'k':
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                curr_item = current_item(menu);
                index = get_book_index(item_name(curr_item));

                if (index == -1)
                    curr_book = node_contains(item_name(curr_item));
                else
                    curr_book = book_data[index];
                break;
            case 'h':
                curr_book->have_read = 1;
                curr_book->in_prog   = 0;
                curr_book->to_read   = 0;
                should_update        = 1;
                break;
            case 'p':
                curr_book->in_prog = !curr_book->to_read;
                should_update      = 1;
                break;
            case 'r':
                curr_book->to_read = !curr_book->to_read;
                should_update      = 1;
                break;
        }

        /* Show the books info on the output window */
        //print_book_info_from_name(output_win, item_name(curr_item));
        if (curr_book)
            print_book_info(output_win, curr_book);
        else
            mvwprintw(output_win, 1, 1, "Error: Cannot find book [%s]", item_name(curr_item));

        print_list_books_help_string(output_win);
    }

    unpost_menu(menu);
}

int main(int argc, char **argv)
{
    init(argc, argv);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    FIELD *field[2];
    FORM  *tag_form;

    ITEM *items[5];
    MENU *main_menu = NULL;

    int ch;

    field[0] = new_field(1, 50, 1, 1, 1, 0);
    field[1] = NULL;

    /* Set field options  */
    set_field_back(field[0], A_UNDERLINE);
    field_opts_off(field[0], O_STATIC);
    set_max_field(field[0], 1023);

    /* Create form */
    tag_form = new_form(field);

    /* Set menu items */
    items[0] = new_item("Check for books", "");
    items[1] = new_item("Get books by tag", "");
    items[2] = new_item("Show books to read", "");
    items[3] = new_item("List books", "");
    items[4] = NULL;


    set_item_userptr(items[0], check_for_books);
    set_item_userptr(items[1], search_for_book);
    set_item_userptr(items[2], list_book_to_read);
    set_item_userptr(items[3], list_books);

    /* Create menu */
    main_menu = new_menu(items);

    /* Create windows */
    WINDOW *main_win   = newwin(20, (COLS / 2), 0, 0);
    WINDOW *output_win = newwin(20, (COLS / 2), 0, (COLS / 2));

    keypad(main_win, TRUE);
    keypad(output_win, TRUE);

    /* Set main window and sub window */
    set_form_win(tag_form, output_win);
    set_form_sub(tag_form, derwin(output_win, 9, 60, 9, 2));

    set_menu_win(main_menu, main_win);
    set_menu_sub(main_menu, derwin(main_win, 9, 60, 2, 2));

    box(main_win, 0, 0);
    box(output_win, 0, 0);

    post_menu(main_menu);

    refresh();
    wrefresh(main_win);
    wrefresh(output_win);

    print_main_menu_help_string(output_win);

    /* Main loop */
    while ((ch = wgetch(main_win)) != ESCAPE_KEY)
    {
        switch (ch)
        {
            case 'j':
            case KEY_DOWN:
                menu_driver(main_menu, REQ_DOWN_ITEM);
                break;
            case 'k':
            case KEY_UP:
                menu_driver(main_menu, REQ_UP_ITEM);
                break;
            case ENTER:
            {
                ITEM *curr_item = current_item(main_menu);

                if (curr_item == items[0])
                {
                    void (*check_for_new_books)(WINDOW*, FORM*, char*) = item_userptr(curr_item);

                    check_for_new_books(output_win, tag_form, books_path);
                    print_main_menu_help_string(output_win);
                }
                else if (curr_item == items[1])
                {
                    unpost_menu(main_menu);
                    void (*search_tags)(WINDOW*, WINDOW*, MENU*, FORM*) = item_userptr(curr_item);

                    search_tags(main_win, output_win, main_menu, tag_form);
                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string(output_win);
                }
                else if (curr_item == items[2])
                {
                    unpost_menu(main_menu);
                    void (*list_to_read)(WINDOW*, WINDOW*, MENU*) = item_userptr(curr_item);

                    list_to_read(main_win, output_win, main_menu);
                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                }
                else if (curr_item == items[3])
                {
                    unpost_menu(main_menu);
                    void (*list_all_books)(WINDOW*, WINDOW*, MENU*, char*) = item_userptr(curr_item);

                    list_all_books(main_win, output_win, main_menu, lib_path);
                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string(output_win);
                }

                pos_menu_cursor(main_menu);
            }
            break;
        }
    }

    unpost_menu(main_menu);

    free_form(tag_form);
    free_field(field[0]);

    free_menu(main_menu);
    for (int i = 0; i < ARR_SIZE(items); i++)
        free_item(items[i]);

    delwin(main_win);
    delwin(output_win);
    endwin();

    if (should_update)
    {
        /* Write book data to the CSV file */
        FILE *fp = fopen(BOOK_DATA_FILE, "w");

        fprintf(fp, "%d\n", book_count + node_count);

        for (int i = 0; i < book_count; i++)
        {
            fprintf(fp, "%s,%d,%d,%d,%s\n", book_data[i]->name,
                                            book_data[i]->have_read,
                                            book_data[i]->in_prog,
                                            book_data[i]->to_read,
                                            book_data[i]->tags);
        }

        for (book_node_t *curr = head; curr; curr = curr->next)
        {
            fprintf(fp, "%s,%d,%d,%d,%s\n", curr->book->name,
                                            curr->book->have_read,
                                            curr->book->in_prog,
                                            curr->book->to_read,
                                            curr->book->tags);
        }

        fclose(fp);
    }

    for (int i = 0; i < book_count; i++)
    {
        free_book(book_data[i]);
    }
    free(book_data);

    for (int i = 0; i < results_count; i++)
    {
        /* The cast from const char* to char* here is a bit of a code smell here */
        /* but this is an easy way to free the memory for the results name as  */
        /* free_item won't free the name or description for us */
        free((char*)results[i]->name.str);
        free_item(results[i]);
    }
    free(results);


    book_node_t *curr = head;
    book_node_t *prev = head;
    while (prev)
    {
        curr = curr->next;
        free_node(prev);

        prev = curr;
    }

    return 0;
}
