BIN = rf
OBJS = rf.o
MANPAGE = rf.1
CC ?= gcc
DEPS = config.h
LIBS =
INC =
CFLAGS := ${CFLAGS}
CFLAGS += -Wall -O2 -fstack-protector-strong -D_FORTIFY_SOURCE=2 $(INC) $(LIBS)
PREFIX ?= /usr/local

build: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(BIN).c $(CFLAGS) -o $(BIN)

%.o : %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

install: build
	install $(BIN) $(DESTDIR)$(PREFIX)/bin/
	install -TD $(MANPAGE) $(DESTDIR)$(PREFIX)/man/man1/$(MANPAGE)
	gzip $(PREFIX)/man/man1/$(MANPAGE)

clean:
	rm -vf $(BIN) *.o
