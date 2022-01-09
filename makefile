CC=gcc
CFLAGS=-Wall -O3
OBJS=book.o dir_utils.o
LIBS=-lncurses -lmenu -lform
PREFIX=/usr/local

all: bookorganiser

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $^

bookorganiser: $(OBJS) main.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	rm $(EXE) $(OBJS) *.csv *.gch

install: output
	mkdir -p $(PREFIX)/bin
	cp -f bookorganiser $(PREFIX)/bin/bookorganiser
	chmod 755 $(PREFIX)/bin/bookorganiser

uninstall:
	rm -f $(PREFIX)/bin/bookorganiser
