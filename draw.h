#ifndef DRAW_H_
#define DRAW_H_

#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>

#include <sys/stat.h>

#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include <readline/readline.h>

#include "book.h"

#define ENTER_KEY  10
#define ESCAPE_KEY 27
#define MIN_WIDTH  64

#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define UNUSED(x) (void)x

void init_draw(char *path);
void cleanup_draw();

void main_loop();

#endif /* DRAW_H_ */
