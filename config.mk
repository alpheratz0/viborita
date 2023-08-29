# Copyright (C) 2023 <alpheratz99@protonmail.com>
# This program is free software.

CC=cc
CFLAGS=-pedantic -Wall -Wextra -Os
LDLIBS_NCURSES=-lcurses
LDLIBS_SDL=-lSDL2 -lSDL2_image -lSDL2_mixer
LDLIBS_XCB=-lxcb -lxcb-keysyms
LDFLAGS=-s
