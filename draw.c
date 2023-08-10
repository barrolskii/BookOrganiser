#include "draw.h"

#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"

#define FILE_UTILS_IMPLEMENTATION
#include "file_utils.h"

/* Defing for additional debugging */
#undef DEBUG_DISPLAY

static FIELD *field[2];
static FORM *tag_form;

#define TOTAL_ITEMS 8
static ITEM *items[TOTAL_ITEMS];
static MENU *main_menu;

/* Item array containing search results */
static ITEM **results = NULL;

static WINDOW *main_win;
static WINDOW *output_win;

/* Flag for whether or not we should update the book records */
/* This will only be true when new books have been added or  */
/* records have been updated */
static int should_update = 0;

/* Window box dimensions */
static int lines   = 0;
static int cols    = 0;
static int start_y = 0;
static int start_x = 0;

static char *books_path          = NULL;
static char share_path[PATH_MAX] = {0};

static dynamic_array *books_list     = NULL;
static dynamic_array *tag_list       = NULL;
static dynamic_array *recently_added = NULL;

/* Input character for readline handling */
static unsigned char input;
static bool input_available = false;
static char completion_append_character = ',';

noreturn static void exit_failure(void)
{
    endwin();
    exit(1);
}

static void clear_previous_results(void)
{
    if (results == NULL)
        return;

    for (unsigned i = 0; results[i]; i++)
    {
        free((char*)results[i]->name.str);
        free_item(results[i]);
    }
    free(results);
    results       = NULL;
}

static int is_in_list(dynamic_array *array, char *name)
{
    if (!array || !name)
        return 0;

   for (size_t i = 0; i < array->size; ++i)
   {
       if (strncmp(array->data[i], name, BOOK_NAME_MAX) == 0)
           return 1;
   }

   return 0;
}

static void read_book_data(char *path)
{
    FILE *fp = fopen(path, "r");

    if (!fp)
    {
        fprintf(stderr, "Unable to open data file when trying to read book data\n");
        exit_failure();
    }

    char *book_data = file_as_str(path);

    if (!book_data)
    {
        fprintf(stderr, "Error: Unable to read data path: %s\n", path);
    }

    cJSON *json_root = cJSON_Parse(book_data);
    cJSON *books = cJSON_GetObjectItem(json_root, "books");

    cJSON *itr = NULL;
    cJSON_ArrayForEach(itr, books)
    {
        book_t *curr_book = deserialise_book(itr);
        if (curr_book == NULL)
        {
            fprintf(stderr, "Unable to deserialise book: %s", itr->valuestring);
            continue;
        }

        dynamic_array_add(books_list, curr_book);

        /* Now add unique tags to the tag list */
        for (size_t i = 0; i < curr_book->tags->size; ++i)
        {
            if (!is_in_list(tag_list, curr_book->tags->data[i]))
            {
                char *copy = strdup(curr_book->tags->data[i]);
                dynamic_array_add(tag_list, copy);
            }
        }
    }

    fclose(fp);
    free(book_data);
    cJSON_Delete(json_root);
}

static void write_book_data(void)
{
    FILE *fp = fopen(share_path, "w+");
    if (!fp)
    {
        fprintf(stderr, "Error: Unable to open book data file %s\n", books_path);
        return;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *books = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "Books", books);

    for (size_t i = 0; i < books_list->size; ++i)
    {
        cJSON *book = serialise_book(books_list->data[i]);
        cJSON_AddItemToArray(books, book);
    }

    char *str = cJSON_Print(root);
    fprintf(fp, "%s", str);

    fclose(fp);
    free(str);
    cJSON_Delete(root);
}

static void print_no_books_message()
{
    werase(output_win);
    box(output_win, 0, 0);
    mvwprintw(output_win, 2, 2, "Unable to list books. No books found");
    wrefresh(output_win);
}

static void print_no_recent_books_message()
{
    werase(output_win);
    box(output_win, 0, 0);
    mvwprintw(output_win, 2, 2, "No new books have been added in this session");
    wrefresh(output_win);
}

static void print_book_info(book_t *book)
{
    assert(book != NULL);

    werase(output_win);

    size_t len = strlen(book->name);

    /* Split the book name on multiple lines if the name is longer than 55 chars */
    /* The limit is 55 chars as this is an integer from dividing 255 (NAME_MAX)  */
    /* by 5. We can get a whole integer by dividing 255 by 3 but this gives us   */
    /* 85 and is a bit too long                                                  */
    for (size_t i = 0, j = 1; j < 5; j++)
    {
        mvwprintw(output_win, j, 1, "%.55s", (book->name + i));

        i += 55;
        if (i > len || i > NAME_MAX)
            break;
    }

    mvwprintw(output_win, 7, 1, "To read:   %d", book->to_read);
    mvwprintw(output_win, 8, 1, "In prog:   %d", book->in_prog);
    mvwprintw(output_win, 9, 1, "Have read: %d", book->have_read);

    for (size_t i = 0; i < book->tags->size; ++i)
        mvwprintw(output_win, 12 + i, 1, "%s", (char*)book->tags->data[i]);

    box(output_win, 0, 0);
    wrefresh(output_win);
}

static void print_list_books_help_string()
{
    mvwprintw(output_win, (lines - 4), 1, "<T> - Update Tags");
    mvwprintw(output_win, (lines - 3), 1, "<R> - Set To Read    <P>   - Set In Progress ");
    mvwprintw(output_win, (lines - 2), 1, "<H> - Set Have Read  <ESC> - Quit");
    wrefresh(output_win);
}

static void print_main_menu_help_string()
{
    mvwprintw(output_win, (lines - 5), 1, "<{>       - Move Up A Page <}>      - Move Down A Page");
    mvwprintw(output_win, (lines - 4), 1, "<g>       - Move To Top    <G>      - Move To Bottom");
    mvwprintw(output_win, (lines - 3), 1, "<J><DOWN> - Move Down      <K><UP>  - Move Up");
    mvwprintw(output_win, (lines - 2), 1, "<ENTER>   - Select Option  <Q><ESC> - Quit");
    wrefresh(output_win);
}

static void trim_field_whitespace(char *field)
{
    for (int i = strlen(field) - 1; field[i] == ' '; --i)
    {
        field[i] = '\0';
    }
}

static dynamic_array *get_new_books(void)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;

    if ((dir = opendir(books_path)) == NULL)
    {
       fprintf(stderr, "Unable to open path [%s] to check for new books\n", books_path);
       return NULL;
    }

    dynamic_array *new_books = dynamic_array_init();

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_REG)
            continue;

        if (is_in_list(books_list, entry->d_name))
            continue;

        book_t *book = init_book();
        strncpy(book->name, entry->d_name, BOOK_NAME_MAX);
        dynamic_array_add(new_books, book);
    }

    closedir(dir);

    return new_books;
}

static dynamic_array *get_books_with_tag(char *search_opt)
{
    dynamic_array *search_results = dynamic_array_init();

    for (size_t i = 0; i < books_list->size; ++i)
    {
        book_t *curr_book = books_list->data[i];

        for (size_t j = 0; j < curr_book->tags->size; ++j)
        {
            if (strstr(curr_book->tags->data[j], search_opt))
                dynamic_array_add(search_results, books_list->data[i]);
        }
    }

    return search_results;
}

static dynamic_array *get_unread_books()
{
    dynamic_array *search_results = dynamic_array_init();

    for (size_t i = 0; i < books_list->size; ++i)
    {
        if (((book_t*)books_list->data[i])->have_read == 0)
            dynamic_array_add(search_results, books_list->data[i]);
    }

    return search_results;
}

static dynamic_array *get_read_books()
{
    dynamic_array *search_results = dynamic_array_init();

    for (size_t i = 0; i < books_list->size; ++i)
    {
        if (((book_t*)books_list->data[i])->have_read == 1)
            dynamic_array_add(search_results, books_list->data[i]);
    }

    return search_results;
}

static dynamic_array *get_books_to_read()
{
    dynamic_array *search_results = dynamic_array_init();

    for (size_t i = 0; i < books_list->size; ++i)
    {
        if (((book_t*)books_list->data[i])->to_read == 1)
            dynamic_array_add(search_results, books_list->data[i]);
    }

    return search_results;
}

static void populate_results(dynamic_array *list)
{
    clear_previous_results();

    results = calloc(list->size + 1, sizeof(ITEM*));

    for (size_t i = 0; i < list->size; ++i)
    {
        char *name = strdup(((book_t*)list->data[i])->name);
        results[i] = new_item(name, "");
    }
}

static void forward_to_readline(char c)
{
    input = c;
    input_available = true;
    rl_callback_read_char();
}

static char *readline_tag_generator(const char *text, int state)
{
    static size_t list_index, len;
    char *name = NULL;

    if (!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    while (list_index < tag_list->size)
    {
        name = tag_list->data[list_index++];

       if (strncmp(name, text, len) == 0)
       {
           return strdup(name);
       }
    }

    return NULL;
}

static char **readline_tag_completion(const char *text, int start, int end)
{
    UNUSED(start);
    UNUSED(end);

    rl_completion_append_character = completion_append_character;
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, readline_tag_generator);
}

static void tag_suggestion_display(char **matches, int size, int len)
{
    UNUSED(len);
    UNUSED(size);

    wmove(output_win, 16, 0);
    wclrtobot(output_win);

    /* First element in matches is the input to match to */
    int i = 1, y_pos= 0;

    for (; matches[i]; ++i, ++y_pos)
    {
        if (y_pos >= lines - 17)
            break;

        mvwprintw(output_win, 16 + y_pos, 2, "|%s|", matches[i]);
    }

    y_pos = 0;

    for (; matches[i]; ++i, ++y_pos)
    {
        if (y_pos >= lines - 17)
            break;

        mvwprintw(output_win, 16 + y_pos, 2 + (cols / 2), "|%s|", matches[i]);
    }

    box(output_win, 0, 0);
    wrefresh(output_win);
}

static int readline_is_input_available(void)
{
    return input_available;
}

static int readline_getc(FILE *dummy)
{
    UNUSED(dummy);

    input_available = false;
    return input;
}

static void redisplay(void)
{
#ifdef DEBUG_DISPLAY
    wmove(output_win, 3, 0);
    clrtoeol();
    wmove(output_win, 4, 2);
    clrtoeol();

    mvwprintw(output_win, 3, 2, "point-%d:end-%d:len-%lu:%s:%s", rl_point, rl_end, strlen(rl_line_buffer), rl_prompt, rl_line_buffer);
    wclrtoeol(output_win);
    mvwprintw(output_win, 4, 2, "%s", callback_handler_str);
#endif

    box(output_win, 0, 0);
    wrefresh(output_win);
}

static void callback_handler(char *line)
{
    static char *prev_line = NULL;

    if (!line)
        return;

    /* Readline will handle cleanup of the memory pointed to by line during teardown */
    /* Only issue being that it won't cleanup any memory if line is a new value */
    /* That's up to this callback function to deal with cleaning the memory up */
    /* So this is why we're freeing prev_line and then assigning it to line to ensure */
    /* that we don't leak any memory */
    free(prev_line);
    prev_line = line;

    redisplay();
}

static void print_suggested_tags()
{
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
        "History and Geography",
        NULL
    };

    for (int i = 0; categories[i]; ++i)
    {
        mvwprintw(output_win, 16 + i, 2, "%s", categories[i]);
    }
}

static void update_book_tags(book_t* curr_book)
{
    char buffer[TAGS_MAX] = {0};
    for (size_t i = 0; i < curr_book->tags->size; ++i)
    {
        strncat(buffer, curr_book->tags->data[i], TAGS_MAX - strlen(buffer));
        strncat(buffer, ",", 2);
    }

    /* Trim the trailing ',' */
    size_t len = strlen(buffer) - 1;
    buffer[len] = '\0';

    werase(output_win);
    box(output_win, 0, 0);
    post_form(tag_form);
    set_field_buffer(tag_form->field[0], 0, buffer);
    rl_insert_text(buffer);

    wrefresh(output_win);

    pos_form_cursor(tag_form);


    int ch;
    while ((ch = wgetch(output_win)))
    {
        if (ch == ENTER_KEY)
        {
            forward_to_readline(ch);
            form_driver(tag_form, REQ_VALIDATION);

            strncpy(buffer, field_buffer(tag_form->field[0], 0), TAGS_MAX - 1);
            trim_field_whitespace(buffer);

            dynamic_array *new_tags = dynamic_array_init();
            char *token = strtok(buffer, ",");

            while (token != NULL)
            {
                char *tag = strdup(token);
                dynamic_array_add(new_tags, tag);

                token = strtok(NULL, ",");
            }

            dynamic_array_clear_user_data(curr_book->tags, free);
            dynamic_array_free(curr_book->tags);
            curr_book->tags = new_tags;

            should_update = 1;

            form_driver(tag_form, REQ_CLR_FIELD);
            unpost_form(tag_form);

            wrefresh(output_win);

            break;
        }

        switch (ch)
        {
            case KEY_LEFT:
            {
                rl_backward_char(1, 0);

                redisplay();
                if (rl_point > 0)
                    form_driver(tag_form, REQ_PREV_CHAR);
                else
                    form_driver(tag_form, REQ_BEG_LINE);

                break;
            }
            case KEY_RIGHT:
            {
                rl_forward_char(1, 0);

                redisplay();
                if (rl_point < rl_end)
                    form_driver(tag_form, REQ_NEXT_CHAR);
                else
                    form_driver(tag_form, REQ_END_LINE);

                break;
            }
            case KEY_BACKSPACE:
                if (rl_point != rl_end)
                {
                    int x, y;
                    getyx(output_win, y, x);

                    rl_backward_char(1, 0);
                    rl_delete(1, 127);

                    form_driver(tag_form, REQ_DEL_PREV);
                    redisplay();

                    wmove(output_win, y, x - 1);
                    break;
                }

                forward_to_readline(127);
                form_driver(tag_form, REQ_DEL_PREV);
                break;
            case KEY_DC:
                forward_to_readline(ch);
                form_driver(tag_form, REQ_DEL_CHAR);
                break;
            case KEY_UP:
                rl_end_of_line(1, 0);
                redisplay();
                form_driver(tag_form, REQ_END_LINE);
                break;
            case KEY_DOWN:
                rl_beg_of_line(1, 0);
                redisplay();
                form_driver(tag_form, REQ_BEG_LINE);
                break;
            case ESCAPE_KEY:
                goto clean_form;
                break;
            case '\t':
            {
                if (rl_point != rl_end)
                {
                    /* Don't allow tab completion if we're not at the end of the word */
                    break;
                }

                forward_to_readline(ch);
                redisplay();
                set_field_buffer(tag_form->field[0], 0, rl_line_buffer);

                form_driver(tag_form, REQ_END_LINE);
                break;
            }
            default:
                if (rl_point != rl_end)
                {
                    int x, y;
                    getyx(output_win, y, x);

                    rl_insert(1, ch);
                    set_field_buffer(tag_form->field[0], 0, rl_line_buffer);
                    redisplay();
                    wmove(output_win, y, x + 1);
                    break;
                }

                forward_to_readline(ch);
                form_driver(tag_form, ch);
        }
    }

clean_form:
    form_driver(tag_form, REQ_CLR_FIELD);
    unpost_form(tag_form);
    wrefresh(output_win);
}

static void menu_handler(dynamic_array *book_array)
{
    int ch;
    size_t index = 0;
    book_t *curr_book = book_array->data[index];

    set_menu_items(main_menu, results);
    post_menu(main_menu);
    pos_menu_cursor(main_menu);

    print_book_info(curr_book);
    print_list_books_help_string();

    while ((ch = wgetch(main_win)))
    {
        if (ch == 'q' || ch == ESCAPE_KEY)
            break;

        switch (ch)
        {

            case 'j':
            case KEY_DOWN:
                menu_driver(main_menu, REQ_DOWN_ITEM);

                ++index;
                if (index >= book_array->size)
                    index = book_array->size - 1;

                curr_book = book_array->data[index];

                break;
            case 'k':
            case KEY_UP:
                menu_driver(main_menu, REQ_UP_ITEM);

                if (index != 0)
                    --index;

                curr_book = book_array->data[index];

                break;
            case 'g':
                menu_driver(main_menu, REQ_FIRST_ITEM);
                break;
            case 'G':
                menu_driver(main_menu, REQ_LAST_ITEM);
                break;
            case '}':
                menu_driver(main_menu, REQ_SCR_DPAGE);
                break;
            case '{':
                menu_driver(main_menu, REQ_SCR_UPAGE);
                break;
            case 'h':
                if (curr_book->have_read)
                {
                    curr_book->have_read = 0;
                    should_update        = 1;
                    break;
                }

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
                update_book_tags(curr_book);
                break;
            }
        }

        print_book_info(curr_book);
        print_list_books_help_string();
    }

    unpost_menu(main_menu);
}

static void check_for_books()
{
    werase(output_win);
    box(output_win, 0, 0);
    wrefresh(output_win);

    dynamic_array *new_books_list = get_new_books();

    if (new_books_list == NULL || new_books_list->size == 0)
    {
        mvwprintw(output_win, 2, 2, "No new books detected");
        print_main_menu_help_string();
        wrefresh(output_win);
        return;
    }

    post_form(tag_form);
    mvwprintw(output_win, 15, 1, "Suggested tags:");
    print_suggested_tags();
    wrefresh(output_win);

    for (size_t i = 0; i < new_books_list->size; ++i)
    {
        /* Clear previous output and update the window with new output */
        wmove(output_win, 1, 1);
        wclrtoeol(output_win);
        box(output_win, 0, 0);
        waddnstr(output_win, new_books_list->data[i], 64);
        mvwprintw(output_win, 12, 1, "%lu/%lu", i + 1, new_books_list->size);

        pos_form_cursor(tag_form);

        int ch;

        while ((ch = wgetch(output_win)))
        {
            if (ch == ENTER_KEY)
            {
                forward_to_readline(ch);
                form_driver(tag_form, REQ_VALIDATION);

                char buffer[TAGS_MAX] = {0};
                strncpy(buffer, field_buffer(tag_form->field[0], 0), TAGS_MAX - 1);
                trim_field_whitespace(buffer);

                /* Add tags to new book */
                char *token = strtok(buffer, ",");
                while (token != NULL)
                {
                    book_t *curr_book = new_books_list->data[i];
                    char *tag = strdup(token);
                    dynamic_array_add(curr_book->tags, tag);

                    if (!is_in_list(tag_list, tag))
                        dynamic_array_add(tag_list, strdup(tag));

                    token = strtok(NULL, ",");
                }

                dynamic_array_add(books_list, new_books_list->data[i]);
                dynamic_array_add(recently_added, new_books_list->data[i]);

                should_update = 1;

                form_driver(tag_form, REQ_CLR_FIELD);
                redisplay();

                break;
            }

            switch (ch)
            {
                case KEY_LEFT:
                {
                    rl_backward_char(1, 0);

                    redisplay();
                    if (rl_point > 0)
                        form_driver(tag_form, REQ_PREV_CHAR);
                    else
                        form_driver(tag_form, REQ_BEG_LINE);

                    break;
                }
                case KEY_RIGHT:
                {
                    rl_forward_char(1, 0);

                    redisplay();
                    if (rl_point < rl_end)
                        form_driver(tag_form, REQ_NEXT_CHAR);
                    else
                        form_driver(tag_form, REQ_END_LINE);

                    break;
                }
                case KEY_BACKSPACE:
                    if (rl_point != rl_end)
                    {
                        int x, y;
                        getyx(output_win, y, x);

                        rl_backward_char(1, 0);
                        rl_delete(1, 127);

                        form_driver(tag_form, REQ_DEL_PREV);
                        redisplay();

                        wmove(output_win, y, x - 1);
                        break;
                    }

                    forward_to_readline(127);
                    form_driver(tag_form, REQ_DEL_PREV);
                    break;
                case KEY_DC:
                    forward_to_readline(ch);
                    form_driver(tag_form, REQ_DEL_CHAR);
                    break;
                case KEY_UP:
                    rl_end_of_line(1, 0);
                    redisplay();
                    form_driver(tag_form, REQ_END_LINE);
                    break;
                case KEY_DOWN:
                    rl_beg_of_line(1, 0);
                    redisplay();
                    form_driver(tag_form, REQ_BEG_LINE);
                    break;
                case ESCAPE_KEY:
                    form_driver(tag_form, REQ_DEL_LINE);
                    rl_delete_text(0, rl_end);
                    /* Cleanup any remaining books */
                    for (; i < new_books_list->size; ++i)
                    {
                        free(new_books_list->data[i]);
                    }
                    goto book_cleanup;
                    break;
                case '\t':
                {
                    if (rl_point != rl_end)
                    {
                        /* Don't allow tab completion if we're not at the end of the word */
                        break;
                    }

                    forward_to_readline(ch);
                    set_field_buffer(tag_form->field[0], 0, rl_line_buffer);
                    form_driver(tag_form, REQ_END_LINE);
                    break;
                }
                default:
                {
                    if (rl_point != rl_end)
                    {
                        int x, y;
                        getyx(output_win, y, x);

                        rl_insert(1, ch);
                        set_field_buffer(tag_form->field[0], 0, rl_line_buffer);
                        redisplay();
                        wmove(output_win, y, x + 1);
                        break;
                    }

                    forward_to_readline(ch);
                    form_driver(tag_form, ch);
                }
            }
        }
    }

    print_main_menu_help_string();
    wrefresh(output_win);


book_cleanup:
    dynamic_array_free(new_books_list);
}

static void search_for_book_by_tag()
{
    werase(output_win);
    box(output_win, 0, 0);
    post_form(tag_form);
    mvwprintw(output_win, 15, 1, "Suggested tags:");
    print_suggested_tags();
    wrefresh(output_win);

    pos_form_cursor(tag_form);

    int ch;
    char search_opt[TAGS_MAX] = {0};

    /* We're updating the append character from ',' to ' ' so we don't mess with the search */
    /* The trailing ' ' character will be trimmed when we pass that string to the search */
    completion_append_character = ' ';

    while ((ch = wgetch(output_win)))
    {
        if (ch == ENTER_KEY)
        {
            forward_to_readline(ch);
            form_driver(tag_form, REQ_VALIDATION);

            strcpy(search_opt, field_buffer(tag_form->field[0], 0));
            trim_field_whitespace(search_opt);

            form_driver(tag_form, REQ_CLR_FIELD);
            unpost_form(tag_form);

            mvwprintw(output_win, 7, 1, "tags: %s", search_opt);
            wrefresh(output_win);

            break;
        }

        switch (ch)
        {
            case KEY_LEFT:
            {
                rl_backward_char(1, 0);

                redisplay();
                if (rl_point > 0)
                    form_driver(tag_form, REQ_PREV_CHAR);
                else
                    form_driver(tag_form, REQ_BEG_LINE);

                break;
            }
            case KEY_RIGHT:
            {
                rl_forward_char(1, 0);

                redisplay();
                if (rl_point < rl_end)
                    form_driver(tag_form, REQ_NEXT_CHAR);
                else
                    form_driver(tag_form, REQ_END_LINE);

                break;
            }
            case KEY_BACKSPACE:
                if (rl_point != rl_end)
                {
                    int x, y;
                    getyx(output_win, y, x);

                    rl_backward_char(1, 0);
                    rl_delete(1, 127);

                    form_driver(tag_form, REQ_DEL_PREV);
                    redisplay();

                    wmove(output_win, y, x - 1);
                    break;
                }

                forward_to_readline(127);
                form_driver(tag_form, REQ_DEL_PREV);
                break;
            case KEY_DC:
                forward_to_readline(ch);
                form_driver(tag_form, REQ_DEL_CHAR);
                break;
            case KEY_UP:
                rl_end_of_line(1, 0);
                redisplay();
                form_driver(tag_form, REQ_END_LINE);
                break;
            case KEY_DOWN:
                rl_beg_of_line(1, 0);
                redisplay();
                form_driver(tag_form, REQ_BEG_LINE);
                break;
            case ESCAPE_KEY:
                completion_append_character = ',';
                return;
            case '\t':
            {
                if (rl_point != rl_end)
                {
                    /* Don't allow tab completion if we're not at the end of the word */
                    break;
                }

                forward_to_readline(ch);
                redisplay();
                set_field_buffer(tag_form->field[0], 0, rl_line_buffer);

                form_driver(tag_form, REQ_END_LINE);
                break;
            }
            default:
            {
                if (rl_point != rl_end)
                {
                    int x, y;
                    getyx(output_win, y, x);

                    rl_insert(1, ch);
                    set_field_buffer(tag_form->field[0], 0, rl_line_buffer);
                    redisplay();
                    wmove(output_win, y, x + 1);
                    break;
                }

                forward_to_readline(ch);
                form_driver(tag_form, ch);
            }
        }
    }

    dynamic_array *search_results = get_books_with_tag(search_opt);

    completion_append_character = ',';

    if (search_results->size == 0)
    {
        print_no_books_message();
        dynamic_array_free(search_results);
        return;
    }

    populate_results(search_results);
    menu_handler(search_results);

    dynamic_array_free(search_results);

    form_driver(tag_form, REQ_CLR_FIELD);
    unpost_form(tag_form);
    wrefresh(output_win);
}

static void list_books_to_read()
{
    dynamic_array *search_results = get_books_to_read();

    if (search_results->size == 0)
    {
        print_no_books_message();
        dynamic_array_free(search_results);
        return;
    }

    populate_results(search_results);
    menu_handler(search_results);

    dynamic_array_free(search_results);
}

static void list_books()
{
    populate_results(books_list);
    menu_handler(books_list);
}

static void list_unread_books()
{
    dynamic_array *search_results = get_unread_books();

    if (search_results->size == 0)
    {
        print_no_books_message();
        dynamic_array_free(search_results);
        return;
    }

    populate_results(search_results);
    menu_handler(search_results);

    dynamic_array_free(search_results);
}

static void list_read_books()
{
    dynamic_array *search_results = get_read_books();

    if (search_results->size == 0)
    {
        print_no_books_message();
        dynamic_array_free(search_results);
        return;
    }

    populate_results(search_results);
    menu_handler(search_results);

    dynamic_array_free(search_results);
}

static void list_recent_books()
{
   if (recently_added->size == 0)
   {
       print_no_recent_books_message();
       return;
   }

    populate_results(recently_added);
    menu_handler(recently_added);
}

static void init_ncurses()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    /* Store the current sessions lines and columns */
    getmaxyx(stdscr, lines, cols);

    if (cols < MIN_WIDTH)
    {
        fprintf(stderr, "Error: Terminal width is too small\n");
        exit_failure();
    }

    /* Decrement lines as we don't want the border to go off the screen */
    lines--;
    cols /= 2;

    /* new_field(height, width, toprow, leftcol, offscreen, nbuffers) */
    /* Width of field is always two thirds of the screen */
    field[0] = new_field(1, ((cols / 3) * 2), 1, 1, 1, 0);
    field[1] = NULL;

    /* Set field options */
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
    items[5] = new_item("Show read books", "");
    items[6] = new_item("Show recent books", "");
    items[7] = NULL;

/* Disabling the pedantic warnings here becuase there's nothing we can do about */
/* the ISO standard of passing a function pointer to a void pointer */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    set_item_userptr(items[0], check_for_books);
    set_item_userptr(items[1], search_for_book_by_tag);
    set_item_userptr(items[2], list_books_to_read);
    set_item_userptr(items[3], list_books);
    set_item_userptr(items[4], list_unread_books);
    set_item_userptr(items[5], list_read_books);
    set_item_userptr(items[6], list_recent_books);
#pragma GCC diagnostic pop

    /* Create menu */
    main_menu = new_menu(items);

    /* Create windows */
    main_win   = newwin(lines, cols, start_y, start_x);
    output_win = newwin(lines, cols, start_y, cols);

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

    print_main_menu_help_string();
}

static void deinit_ncurses()
{
    unpost_menu(main_menu);

    free_form(tag_form);
    free_field(field[0]);

    free_menu(main_menu);
    for (size_t i = 0; i < ARR_SIZE(items); ++i)
        free_item(items[i]);


    delwin(main_win);
    delwin(output_win);
    endwin();
}

static void init_readline()
{
    rl_catch_signals  = 0;
    rl_catch_sigwinch = 0;

    rl_prep_term_function   = NULL;
    rl_deprep_term_function = NULL;

    rl_basic_word_break_characters = ",";

    /* Prevent readline from changing the LINES and COLUMNS environment variables */
    rl_change_environment = 0;

    rl_getc_function = readline_getc;
    rl_input_available_hook = readline_is_input_available;
    rl_redisplay_function = redisplay; /* We need a redisplay function otherwise callback handler install will crash */

    rl_attempted_completion_function = readline_tag_completion;
    rl_completion_display_matches_hook = tag_suggestion_display;

    rl_callback_handler_install("> ", callback_handler);
}

static void deinit_readline()
{
    rl_callback_handler_remove();
}

void init_draw(char *path)
{
    if (access(path, F_OK) != 0)
    {
        fprintf(stderr, "Unable to find provided path: %s\n", path);
        exit_failure();
    }

    books_list = dynamic_array_init();
    tag_list = dynamic_array_init();
    recently_added = dynamic_array_init();

    strcat(share_path, getenv("HOME"));
    strcat(share_path, "/.local/share/bookorganiser");

    if (access(share_path, F_OK) != 0)
    {
        if (mkdir(share_path, S_IRWXU | S_IRGRP | S_IXGRP) != 0)
        {
            fprintf(stderr, "Error unable to create %s", share_path);
            exit_failure();
        }
    }

    strcat(share_path, "/data.json");
    if (access(share_path, F_OK) == 0)
    {
        read_book_data(share_path);
    }
    else
    {
        FILE *fp = fopen(share_path, "w");

        if (!fp)
        {
            fprintf(stderr, "Unable to create data json file\n");
            exit_failure();
        }

        fclose(fp);
    }

    books_path = path;

    init_ncurses();
    init_readline();
}

void cleanup_draw()
{
    if (should_update)
        write_book_data();

    deinit_readline();
    deinit_ncurses();

    dynamic_array_clear_user_data(books_list, free_book);
    dynamic_array_free(books_list);

    dynamic_array_clear_user_data(tag_list, free);
    dynamic_array_free(tag_list);

    dynamic_array_free(recently_added);

    clear_previous_results();
}

void main_loop()
{
    int ch;

    while ((ch = wgetch(main_win)))
    {
        if (ch == 'q' || ch == ESCAPE_KEY)
            break;

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
            case 'g':
                menu_driver(main_menu, REQ_FIRST_ITEM);
                break;
            case 'G':
                menu_driver(main_menu, REQ_LAST_ITEM);
                break;
            case ENTER_KEY:
            {
                ITEM *curr_item = current_item(main_menu);
                pos_menu_cursor(main_menu);

                if (curr_item == items[0])
                {
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wpedantic"

                    void (*check_for_new_books)() = item_userptr(curr_item);
                    check_for_new_books();

                    #pragma GCC diagnostic pop

                    print_main_menu_help_string();
                }

                /* Print the no book message and stop the user from trying to use any */
                /* other menu option. No point in trying when there are no books      */
                if (books_list->size == 0)
                {
                    print_no_books_message();
                    break;
                }

                if (curr_item == items[1])
                {
                    unpost_menu(main_menu);

                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wpedantic"

                    void (*search_tags)() = item_userptr(curr_item);
                    search_tags();

                    #pragma GCC diagnostic pop

                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string();
                }
                else if (curr_item == items[2])
                {
                    unpost_menu(main_menu);

                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wpedantic"

                    void (*list_books_to_read)() = item_userptr(curr_item);
                    list_books_to_read();

                    #pragma GCC diagnostic pop

                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string();
                }
                else if (curr_item == items[3])
                {
                    unpost_menu(main_menu);


                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wpedantic"

                    void (*list_all_books)() = item_userptr(curr_item);
                    list_all_books();

                    #pragma GCC diagnostic pop

                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string();
                }
                else if (curr_item == items[4])
                {
                    unpost_menu(main_menu);

                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wpedantic"

                    void (*list_unread_books)() = item_userptr(curr_item);
                    list_unread_books();

                    #pragma GCC diagnostic pop

                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string();
                }
                else if (curr_item == items[5])
                {
                    unpost_menu(main_menu);

                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wpedantic"

                    void (*list_read_books)() = item_userptr(curr_item);
                    list_read_books();

                    #pragma GCC diagnostic pop

                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string();
                }
                else if (curr_item == items[6])
                {
                    unpost_menu(main_menu);

                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wpedantic"

                    void (*list_recent_books)() = item_userptr(curr_item);
                    list_recent_books();

                    #pragma GCC diagnostic pop

                    set_menu_items(main_menu, items);
                    post_menu(main_menu);
                    print_main_menu_help_string();
                }
            }
            break;
        }
    }

}
