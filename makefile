CC=gcc
CFLAGS=-Wall -O3
OBJS=book.o dir_utils.o
LIBS=-lncurses -lmenu -lform

all: bookorganiser

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $^

book_organiser: $(OBJS) main.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	rm $(EXE) $(OBJS) *.csv *.gch

