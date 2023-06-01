.POSIX:
.PHONY: all clean

include config.mk

all: viborita_ncurses viborita_sdl

viborita_ncurses: main_ncurses.c map.c util.c
	$(CC) $(LDFLAGS) -o $@ main_ncurses.c map.c util.c $(LDLIBS_NCURSES)

viborita_sdl: main_sdl.c map.c util.c
	$(CC) $(LDFLAGS) -o $@ main_sdl.c map.c util.c $(LDLIBS_SDL)

clean:
	rm -f viborita_ncurses viborita_sdl
