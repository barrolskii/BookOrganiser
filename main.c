#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <argp.h>
#include <argz.h>

#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include <sys/stat.h>

#include "defines.h"
#include "book.h"
#include "dir_utils.h"

char books_path[PATH_MAX]     = {0};
char lib_path[PATH_MAX]       = {0};
char data_file_path[PATH_MAX] = {0};

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

/* Window box dimensions */
int lines   = 0;
int cols    = 0;
int start_y = 0;
int start_x = 0;

char *categories[] = {
    "Computer Science",
    "Philosophy and Psychology",
    "Religion",
    "Social Sciences",
    "Language",
    "Science",
    "Technology",
    "Arts and Recreation",
    "Literature",
    "History and Geography"
};

/* Globals needed for argument parsing with argp/argz */
struct arguments {
    char *argz;
    size_t len;
};

static struct argp_option options[] = {
    { "path", 'p', "STRING", 0, "Path to directory containing books", 0},
    {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);

const char *argp_program_version = VERSION;
static struct argp argp = {options, parse_opt, "path", VERSION, NULL, NULL, NULL};

void clear_previous_results(void)
{
    /* Cleanup the previous results */
    if (results != NULL)
    {
        for (unsigned i = 0; i < results_count; i++)
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
    mvwprintw(win, (lines - 3), 1, "<J><DOWN> - Move Down      <K><UP> - Move Up");
    mvwprintw(win, (lines - 2), 1, "<ENTER>   - Select Option  <ESC> - Quit");
    wrefresh(win);
}

void print_list_books_help_string(WINDOW *win)
{
    mvwprintw(win, (lines - 3), 1, "<R> - Set To Read    <P>   - Set In Progress ");
    mvwprintw(win, (lines - 2), 1, "<H> - Set Have Read  <ESC> - Quit");
    wrefresh(win);
}

int get_book_index(const char *name)
{
    for (unsigned i = 0; i < book_count; i++)
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

    size_t len = strlen(book->name);

    /* Split the book name on multiple lines if the name is longer than 55 chars */
    /* The limit is 55 chars as this is an integer from dividing 255 (NAME_MAX)  */
    /* by 5. We can get a whole integer by dividing 255 by 3 but this gives us   */
    /* 85 and is a bit too long                                                  */
    for (size_t i = 0, j = 1; j < 5; j++)
    {
        mvwprintw(win, j, 1, "%.55s", (book->name + i));

        i += 55;
        if (i > len || i > NAME_MAX)
            break;
    }

    mvwprintw(win, 7, 1, "%s",             book->tags);
    mvwprintw(win, 11, 1, "To read:   %d", book->to_read);
    mvwprintw(win, 12, 1, "In prog:   %d", book->in_prog);
    mvwprintw(win, 13, 1, "Have read: %d", book->have_read);

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

        print_book_info(win, book);
    }

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

void trim_field_whitespace(char *field)
{
        for (int i = strlen(field); i > 0; i--)
        {
            if ((field[i] >= 33) && (field[i] <= 126))
            {
                field[i + 1] = '\0';
                break;
            }
        }
}

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *a = (struct arguments*)state->input;

    switch (key)
    {
        case 'p':
        {
            argz_add(&a->argz, &a->len, arg);
            if (strlen(arg) >= PATH_MAX)
            {
                fprintf(stderr, "Error: Provided path is too long. Path must be shorter than %d\n", PATH_MAX);
                exit(1);
            }
            strcpy(books_path, arg);
            break;
        }
        case ARGP_KEY_ARG:
            argz_add(&a->argz, &a->len, arg);
            break;
        case ARGP_KEY_INIT:
            a->argz = 0;
            a->len = 0;
            break;
        case ARGP_KEY_END:
        {
            size_t count = argz_count(a->argz, a->len);
            if (count < 1)
                argp_failure(state, 1, 0, "too few arguments");
            else if (count > 1)
                argp_failure(state, 1, 0, "too many arguments");
        }
        break;
    }

    return 0;
}

void init(int argc, char **argv)
{
    struct arguments args = {0};
    if (argp_parse(&argp, argc, argv, 0, 0, &args) == 0)
    {
        const char *prev_arg = NULL;
        char *curr_arg;

        while ((curr_arg = argz_next (args.argz, args.len, prev_arg)))
        {
            if (strcmp(books_path, "") == 0)
                strcpy(books_path, curr_arg);

            prev_arg = curr_arg;
        }

        free(args.argz);
    }
    else
    {
        fprintf(stderr, "Error: argp_parse is unable to parse command arguments");
        exit(1);
    }

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

    /* Create the path for the book data file */
    const char *homedir = getenv("HOME");
    strcpy(data_file_path, homedir);
    strcat(data_file_path, "/");
    strcat(data_file_path, BOOK_DATA_PATH);

    /* Create the book organiser directory first before we try */
    if (!dir_exists(data_file_path))
    {
        if (mkdir(data_file_path, S_IRWXU) != 0)
        {
            fprintf(stderr, "Error: Failed to create directory for bookorganiser data file\n");
            fprintf(stderr, "%s\n", strerror(errno));
        }
    }

    strcat(data_file_path, BOOK_DATA_FILE);

    /* Check if data file for books exists */
    if (access(data_file_path, E_OK) == 0)
    {
        FILE *fp = fopen(data_file_path, "r");

        char name[1024] = {0};
        char tags[1024] = {0};
        char have_read;
        char in_prog;
        char to_read;

        fscanf(fp, "%u\n", &book_count);


        book_data = calloc(book_count, sizeof(book_t*));
        for (unsigned i = 0; i < book_count; i++)
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
        FILE *file = fopen(data_file_path, "w+");
        if (file == NULL)
        {
            fprintf(stderr, "Error: Unable to create file for book data in path %s\n", data_file_path);
            fprintf(stderr, "%s\n", strerror(errno));
            exit(1);
        }
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

    mvwprintw(win, 12, 1, "Suggested tags:");
    mvwprintw(win, 13, 4,  "%s",   categories[0]);
    mvwprintw(win, 13, 26, "%s",   categories[1]);
    mvwprintw(win, 14, 4,  "%s",   categories[2]);
    mvwprintw(win, 14, 26, "%s",   categories[3]);
    mvwprintw(win, 15, 4,  "%s",   categories[4]);
    mvwprintw(win, 15, 26, "%s",   categories[5]);
    mvwprintw(win, 16, 4,  "%s",   categories[6]);
    mvwprintw(win, 16, 26, "%s",   categories[7]);
    mvwprintw(win, 17, 4,  "%s",   categories[8]);
    mvwprintw(win, 17, 26, "%s",   categories[9]);

    wrefresh(win);

    char old_path[MAX_LENGTH] = {0};
    char new_path[MAX_LENGTH] = {0};

    int ch;

    for (unsigned i = 0; i < count; i++)
    {
        /* Clear previous output and update the window with new output */
        wmove(win, 1, 1);
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

            trim_field_whitespace(search_opt);

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

    for (unsigned i = 0; i < book_count; i++)
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

        unsigned i = 0, j = 0;
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

void list_book_to_read(WINDOW *menu_win, WINDOW *output_win, MENU *menu, int to_read)
{
    clear_previous_results();

    for (unsigned i = 0; i < book_count; i++)
    {
        if (book_data[i]->to_read == to_read)
            ++results_count;
    }


    for (book_node_t *node = head; node; node = node->next)
    {
        if (node->book->to_read == to_read)
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

        unsigned i = 0, j = 0;
        for (; i < book_count; i++)
        {
            if (book_data[i]->to_read == to_read)
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
            if (node->book->to_read == to_read)
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
    ITEM *curr_item   = NULL;
    book_t *curr_book = NULL;

    set_menu_items(menu, results);
    post_menu(menu);
    pos_menu_cursor(menu);

    curr_item = current_item(menu);
    print_book_info_from_name(output_win, item_name(curr_item));
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
        }

        if (curr_book)
            print_book_info(output_win, curr_book);
        else
            mvwprintw(output_win, 1, 1, "Error: Cannot find book [%s]", item_name(curr_item));

        print_list_books_help_string(output_win);
    }

    unpost_menu(menu);
}

void list_books(WINDOW *menu_win, WINDOW *output_win, MENU *menu, FORM *form, char *path)
{
    clear_previous_results();

    results_count = dir_count_files(path);

    if (results_count == 0)
    {
        werase(output_win);
        box(output_win, 0, 0);
        mvwprintw(output_win, 2, 2, "Unable to list books. No books found");
        wrefresh(output_win);
        return;
    }

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

    if (curr_book == NULL)
    {
        mvwprintw(output_win, 1, 1, "Error: Cannot find book [%s]", item_name(curr_item));
    }
    else
    {
        print_book_info(output_win, curr_book);
        print_list_books_help_string(output_win);
    }

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
            case 't':
            {
                int ch;

                werase(output_win);
                box(output_win, 0, 0);

                set_field_buffer(form->field[0], 0, curr_book->tags);
                post_form(form);
                wrefresh(output_win);

                pos_form_cursor(form);

                while ((ch = wgetch(output_win)))
                {
                    if (ch == ENTER)
                    {
                        form_driver(form, REQ_VALIDATION);

                        if (strcmp(curr_book->tags, field_buffer(form->field[0], 0)) != 0)
                        {
                            char buffer[1024] = {0};
                            strcpy(buffer, field_buffer(form->field[0], 0));
                            trim_field_whitespace(buffer);

                            int len = strlen(buffer) + 1;

                            curr_book->tags = realloc(curr_book->tags, (sizeof(char) * len));
                            curr_book->tags[len - 1] = '\0';

                            /* Update the current books tags to be the new tags */
                            strcpy(curr_book->tags, buffer);
                            should_update = 1;
                        }

                        form_driver(form, REQ_CLR_FIELD);
                        unpost_form(form);

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
                            goto clean_form;
                            break;
                        default:
                            form_driver(form, ch);
                    }
                }

                clean_form:
                form_driver(form, REQ_CLR_FIELD);
                unpost_form(form);
                wrefresh(output_win);

                break;
            }
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

    /* Store the current sessions lines and columns */
    getmaxyx(stdscr, lines, cols);

    if (cols < MIN_WIDTH)
    {
        endwin();
        fprintf(stderr, "Error: Terminal width is too small\n");
        goto cleanup;
    }

    /* Decrement lines as we don't want the border to go off the screen */
    lines--;
    cols /= 2;


    FIELD *field[2];
    FORM  *tag_form;

    ITEM *items[6];
    MENU *main_menu = NULL;

    int ch;

    field[0] = new_field(1, ((cols / 3) * 2), 1, 1, 1, 0);
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
    items[4] = new_item("Show unread books", "");
    items[5] = NULL;

    set_item_userptr(items[0], check_for_books);
    set_item_userptr(items[1], search_for_book);
    set_item_userptr(items[2], list_book_to_read);
    set_item_userptr(items[3], list_books);
    set_item_userptr(items[4], list_book_to_read);

    /* Create menu */
    main_menu = new_menu(items);

    /* Create windows */
    WINDOW *main_win   = newwin(lines, cols, start_y, start_x);
    WINDOW *output_win = newwin(lines, cols, start_y, cols);

    keypad(main_win, TRUE);
    keypad(output_win, TRUE);

    /* Set main window and sub window */
    set_form_win(tag_form, output_win);
    set_form_sub(tag_form, derwin(output_win, 9, ((COLS / 2) - 10), 9, 2)); /* orig, nlines, ncols, begin_y, begin_x */

    set_menu_win(main_menu, main_win);
    set_menu_sub(main_menu, derwin(main_win, (lines - 3), ((COLS / 2) - 10), 2, 2));
    set_menu_format(main_menu, (lines - 3), 1);

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
                    void (*list_to_read)(WINDOW*, WINDOW*, MENU*, int) = item_userptr(curr_item);

                    list_to_read(main_win, output_win, main_menu, 1);
                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string(output_win);
                }
                else if (curr_item == items[3])
                {
                    unpost_menu(main_menu);
                    void (*list_all_books)(WINDOW*, WINDOW*, MENU*, FORM*, char*) = item_userptr(curr_item);

                    list_all_books(main_win, output_win, main_menu, tag_form, lib_path);
                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string(output_win);
                }
                else if (curr_item == items[4])
                {
                    unpost_menu(main_menu);
                    void (*list_to_read)(WINDOW*, WINDOW*, MENU*, int) = item_userptr(curr_item);

                    list_to_read(main_win, output_win, main_menu, 0);
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
    for (size_t i = 0; i < ARR_SIZE(items); i++)
        free_item(items[i]);

    delwin(main_win);
    delwin(output_win);
    endwin();

cleanup:
    if (should_update)
    {
        /* Write book data to the CSV file */
        FILE *fp = fopen(data_file_path, "w");

        fprintf(fp, "%d\n", book_count + node_count);

        for (unsigned i = 0; i < book_count; i++)
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

    for (unsigned i = 0; i < book_count; i++)
    {
        free_book(book_data[i]);
    }
    free(book_data);

    for (unsigned i = 0; i < results_count; i++)
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
