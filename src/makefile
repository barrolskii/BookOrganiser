exe: main.o utils.o
	gcc main.o utils.o -lreadline -l:libkoios.a -lm -o exe

main.o: main.c
	gcc -c main.c

utils.o: utils.c utils.h
	gcc -c utils.c

clean:
	rm *.o exe
