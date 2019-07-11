build:
	gcc -g -c utility.c -o utility.o
	gcc -g -c main.c -o main.o
	gcc -g -o exe utility.o main.o

clean:
	rm utility.o main.o
