all:
	cd src; make
	mv src/sqlbak .

install: all
	install -d $(DESTDIR)/$(PREFIX)/bin/
	install sqlbak $(DESTDIR)/$(PREFIX)/bin/

clean:
	cd src; make clean
