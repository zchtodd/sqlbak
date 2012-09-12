CFLAGS=-Wall -g
LOADLIBES=-lsqlite3 -ltar

all: sqlbak

sqlbak: log.o fswalk.o backup.o 

install: all
	install -d $(DESTDIR)/$(PREFIX)/bin/
	install sqlbak $(DESTDIR)/$(PREFIX)/bin/

clean:
	rm -f *.o
	rm -f sqlbak
