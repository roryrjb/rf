.POSIX:

BIN = rf
VERSION = 0.0.5
OBJS = rf.o ignore.o
PREFIX = /usr/local
CC = cc
CFLAGS = -std=c99 -O2 \
	  -Wpedantic -Wall \
	  -fstack-protector-strong -fpie \
	  -D_FORTIFY_SOURCE=2 \
	  -DVERSION='"$(VERSION)"' \
	  -DNAME='"$(BIN)"'

build: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS)

install: build
	mkdir -p \
		$(DESTDIR)$(PREFIX)/bin \
		$(DESTDIR)$(PREFIX)/man/man1 \
		$(DESTDIR)$(PREFIX)/man/man5
	install -m755 $(BIN) $(DESTDIR)$(PREFIX)/bin/
	install -m444 rf.1 $(DESTDIR)$(PREFIX)/man/man1/
	install -m444 rfignore.5 $(DESTDIR)$(PREFIX)/man/man5/

clean:
	rm -vf $(BIN) *.o
