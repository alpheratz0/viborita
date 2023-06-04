/*
	Copyright (C) 2023 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License version 2 as published by the
	Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include "map.h"
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

int
main(int argc, char **argv)
{
	int score = 0;
	struct map map;
	enum map_viborita_state state;
	char map_str[MAX_MAP_STR_LEN+1];
	int c;

	if (argc < 2 || map_parse_file(&map, argv[1]) < 0)
	{
		fprintf(stderr, "usage: viborita_ncurses <valid_map_path>\n");
		return 1;
	}

	initscr();
	nodelay(stdscr, TRUE);
	noecho();

	do
	{
		while ((c = getch()) != ERR) switch (c)
		{
			case 'h': map_set_viborita_direction(&map, MAP_BLOCK_VIBORITA_LEFT); break;
			case 'j': map_set_viborita_direction(&map, MAP_BLOCK_VIBORITA_DOWN); break;
			case 'k': map_set_viborita_direction(&map, MAP_BLOCK_VIBORITA_UP); break;
			case 'l': map_set_viborita_direction(&map, MAP_BLOCK_VIBORITA_RIGHT); break;
		}

		map_advance(&map, &state);

		if (state == MAP_VIBORITA_EATING)
		{
			map_spawn_food(&map);
			score += 1;
		}

		map_stringify(&map, sizeof(map_str), map_str);
		move(0, 0);
		printw(map_str);
		refresh();

		usleep(100000);
	}
	while (state != MAP_VIBORITA_DEAD);

	endwin();
	printf("xviborita score: %d\n", score);

	return 0;
}
