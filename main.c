#include <stdio.h>

#include "terminalColors.h"

int main(int argc, char **argv)
{
	printf(ANSI_COLOR_GREEN "Hello world\n" ANSI_COLOR_RESET);

	return 0;
}
