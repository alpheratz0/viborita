.POSIX:
.PHONY: all clean

include config.mk

all: xviborita_ncurses

xviborita_ncurses: main_ncurses.c map.c util.c
	$(CC) $(LDFLAGS) -o $@ main_ncurses.c map.c util.c $(LDLIBS)

clean:
	rm -f xviborita_ncurses
