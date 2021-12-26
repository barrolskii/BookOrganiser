CC=gcc
CFLAGS=-Wall -g
OBJS=
LIBS=-lncurses -lmenu -lform
EXE=prog

$(EXE): main.c
	$(CC) $(CFLAGS) $(LIBS) main.c -o $(EXE)

clean:
	rm $(EXE)

