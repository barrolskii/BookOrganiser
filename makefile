build:
	gcc -g -c utility.c -o utility.o
	gcc -g -c book.c -o book.o
	gcc -g -c database.c -o database.o
	gcc -g -c main.c -o main.o
	gcc -g -o exe utility.o book.o database.o main.o

clean:
	rm utility.o book.o database.o main.o
