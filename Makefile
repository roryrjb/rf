.POSIX:
.SUFFIXES: .c .o

BIN = rf
OBJS = rf.o
MANPAGE = rf.1
CC = cc
DEPS = config.h
LIBS =
INC =
CFLAGS := ${CFLAGS}
CFLAGS += -ansi \
	  -Wpedantic \
	  -Wall \
	  -Werror=format-security \
	  -Werror=implicit-function-declaration \
	  -O2 \
	  -fstack-protector-strong \
	  -fpie \
	  -D_FORTIFY_SOURCE=2 $(INC) $(LIBS)
PREFIX ?= /usr/local

build: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(BIN).c $(CFLAGS) -o $(BIN)

debug: $(OBJS)
	$(CC) $(BIN).c $(CFLAGS) -g -o $(BIN)

test: clean debug
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./$(BIN) ^$
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./$(BIN) ^$$
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./$(BIN) rf

static: $(OBJS)
	$(CC) $(BIN).c $(CFLAGS) -static -o $(BIN)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

install: build
	install $(BIN) $(DESTDIR)$(PREFIX)/bin/
	install -TD $(MANPAGE) $(DESTDIR)$(PREFIX)/man/man1/$(MANPAGE)
	gzip $(PREFIX)/man/man1/$(MANPAGE)

clean:
	rm -vf $(BIN) *.o
