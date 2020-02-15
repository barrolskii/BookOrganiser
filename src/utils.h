/* Print utility functions */
unsigned int get_longest_strlen(char **arr, int len);
void print_str_array(char **arr, int len);
void print_border(void (*fun)(char**, int), char **arr, int arr_len);

/* Directory utility functions */
int check_directory_exists(char *dir_name);
int create_directory(char *name);
unsigned int count_files_in_directory(char *path);

