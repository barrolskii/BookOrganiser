#include "book.h"
#include "draw.h"

#include "file_utils.h"
#include "dynamic_array.h"

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        fprintf(stderr, "Error: Too many arguments supplied\n");
        return -1;
    }

    if (argc < 2)
    {
        fprintf(stderr, "Error: No path supplied\n");
        return -1;
    }


    init_draw(argv[1]);
    main_loop();
    cleanup_draw();

    return 0;
}
