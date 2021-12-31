CC=gcc
CFLAGS=-Wall -O3
OBJS=
LIBS=-lncurses -lmenu -lform
EXE=prog

$(EXE): main.c
	$(CC) $(CFLAGS) $(LIBS) main.c -o $(EXE)

clean:
	rm $(EXE) *.csv

