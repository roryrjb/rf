.POSIX:

BIN = rf
VERSION = 0.0.5
OBJS = rf.o ignore.o config.o
PREFIX = /usr/local
CC = cc
CFLAGS = -std=c99 -pedantic -O2 \
	  -Wall -Wextra -Wsign-compare \
	  -fstack-protector-strong -fpie \
	  -D_FORTIFY_SOURCE=2 \
	  -DVERSION='"$(VERSION)"' \
	  -DNAME='"$(BIN)"'

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS)

rf.1: rf.1.scd
	scdoc < $< > $@

rfignore.5: rfignore.5.scd
	scdoc < $< > $@

rfconfig.5: rfconfig.5.scd
	scdoc < $< > $@

install: $(BIN) rf.1 rfignore.5 rfconfig.5
	mkdir -p \
		$(DESTDIR)$(PREFIX)/bin \
		$(DESTDIR)$(PREFIX)/man/man1 \
		$(DESTDIR)$(PREFIX)/man/man5
	install -m755 $(BIN) $(DESTDIR)$(PREFIX)/bin/
	install -m444 rf.1 $(DESTDIR)$(PREFIX)/man/man1/
	install -m444 rfignore.5 $(DESTDIR)$(PREFIX)/man/man5/
	install -m444 rfconfig.5 $(DESTDIR)$(PREFIX)/man/man5/

clean:
	rm -vf $(BIN) *.o *.1 *.5
