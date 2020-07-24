PREFIX = /usr/local

output: main.o utils.o
	gcc main.o utils.o -lreadline -l:libkoios.a -lm -o bookorganiser

main.o: main.c
	gcc -c main.c

utils.o: utils.c utils.h
	gcc -c utils.c

clean:
	rm *.o bookorganiser

install: output
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f bookorganiser $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/bookorganiser

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/bookorganiser
