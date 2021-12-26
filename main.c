#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>

#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include <sys/stat.h>

#define ENTER 10

#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

#define FREE_2D_ARR(arr, size)     \
    for (int i = 0; i < size; i++) \
        free(arr[i]);              \
    free(arr);                     \

// TODO: Helper file
int dir_exists(char *path)
{
    DIR *dir = NULL;

    if ((dir = opendir(path)) != NULL)
    {
        closedir(dir);
        return 1;
    }

    printf("no\n");
    printf("dir %s\n", path);
    fprintf(stderr, "Error: opendir() failed with error message [%s]\n", strerror(errno));

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
        files[i] = calloc(FILENAME_MAX, sizeof(char));
    }

    DIR *dir = NULL;
    struct dirent *entry = NULL;

    if ((dir = opendir(path)) != NULL)
    {
        // TODO: For loop this
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type != DT_REG)
                continue;

            memcpy(files[i], entry->d_name, FILENAME_MAX);
            i++;
        }
    }

    return files;
}

void init(void)
{
    // TODO: Args
    char *lib_path = "/home/ash/Documents/Books/library";
    char *path = "/home/ash/Documents/Books";

    // TODO: Define this
    if (strlen(path) > 1024)
    {
        fprintf(stderr, "Error: Provided path [%s] is too long.\n Max supported path is 255 bytes\n", path);
        exit(1);
    }

    if (!dir_exists(path))
    {
        fprintf(stderr, "Error: Provided path [%s] does not exist.\n", path);
        exit(1);
    }

    if (!dir_exists(lib_path))
    {
        if (mkdir(lib_path, S_IRWXU) != 0)
        {
            fprintf(stderr, "Error: Failed to create library path [%s].\n", lib_path);
            fprintf(stderr, "mkdir failed with error message [%s]\n", strerror(errno));
        }
    }
}

typedef struct {
    char *name;
    char **tags;
} book_t;

void check_for_books(WINDOW *win, FORM *form)
{
    printw("YES");
    refresh();
}

int main(int argc, char **argv)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    FIELD *field[2];
    FORM  *tag_form;

    ITEM *items[5];
    MENU *main_menu = NULL;

    int ch;

    field[0] = new_field(1, 10, 1, 1, 0, 0);
    field[1] = NULL;

    /* Set field options  */
    set_field_back(field[0], A_UNDERLINE);
    //field_opts_off(field[0], O_AUTOSKIP); /* Don't go to next field when this */

    /* Create form */
    tag_form = new_form(field);

    /* Set menu items */
    items[0] = new_item("Check for books", "");
    items[1] = new_item("Get books by tag", "");
    items[2] = new_item("Show books to read", "");
    items[3] = new_item("List books", "");
    items[4] = NULL;


    set_item_userptr(items[0], check_for_books);
    set_item_userptr(items[1], NULL);
    set_item_userptr(items[2], NULL);
    set_item_userptr(items[3], NULL);

    /* Create menu */
    main_menu = new_menu(items);

    /* Create windows */
    WINDOW *main_win   = newwin(20, (COLS / 2), 0, 0);
    WINDOW *output_win = newwin(20, (COLS / 2), 0, (COLS / 2));

    keypad(main_win, TRUE);
    keypad(output_win, TRUE);

    /* Set main window and sub window */
    set_form_win(tag_form, output_win);
    set_form_sub(tag_form, derwin(output_win, 9, 11, 2, 2));

    set_menu_win(main_menu, main_win);
    set_menu_sub(main_menu, derwin(main_win, 9, 11, 2, 2));

    box(main_win, 0, 0);
    box(output_win, 0, 0);

    post_menu(main_menu);

    refresh();
    wrefresh(main_win);
    wrefresh(output_win);

    /* Main loop */
    while ((ch = wgetch(main_win)) != 27)
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
                    void (*func)(WINDOW*, FORM*) = item_userptr(curr_item);
                    func(output_win, tag_form);
                }
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

    return 0;
}
