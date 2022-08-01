.POSIX:

VERSION = 0.1
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

PKG_CONFIG = pkg-config

BCFLAGS = $(CFLAGS)
BLDFLAGS = `$(PKG_CONFIG) --cflags --libs json-c libcurl` -lm

SRC_DIR = $(shell pwd)
SRCS = $(wildcard *.c)
OBJ = $(SRCS:%.c=%.o)

all: options ctm

options:
	@echo ctm build options:
	@echo "CFLAGS   = $(BCFLAGS)"
	@echo "LDFLAGS  = $(BLDFLAGS)"
	@echo "CC       = $(CC)"

.c.o:
	$(CC) -c $(BLDFLAGS) $<

ctm: $(OBJ)
	$(CC) -o $@ $(OBJ) $(BCFLAGS) $(BLDFLAGS)

clean:
	rm -f ctm $(OBJ)

install: ctm
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	mkdir -p $(DESTDIR)$(PREFIX)/share/bash-completion/completions
	mkdir -p $(DESTDIR)$(PREFIX)/share/zsh/site-functions
	cp -f ctm $(DESTDIR)$(PREFIX)/bin
	cp -f ctm.1 $(DESDIR)$(MANPREFIX)/man1/ctm.1
	chmod 775 $(DESTDIR)$(PREFIX)/bin/ctm
	cp -f completion/bash/ctm.bash $(DESTDIR)$(PREFIX)/share/bash-completion/completions/timestamp
	cp -f completion/zsh/_ctm $(DESTDIR)$(PREFIX)/share/zsh/site-functions

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/ctm
	rm -f $(DESKDIR)$(MANPREFIX)/man1/ctm.1
	rm -f $(DESTDIR)$(PREFIX)/share/bash-completion/completions/ctm
	rm -f $(DESTDIR)$(PREFIX)/share/zsh/site-functions/_ctm

.PHONY: all options ctm clean install uninstall
