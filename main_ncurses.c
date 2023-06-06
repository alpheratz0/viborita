/*
	Copyright (C) 2023 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License version 2 as published by
	the Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
	more details.

	You should have received a copy of the GNU General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include "map.h"
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdbool.h>

#define PAUSE_MSG "paused"

int
main(int argc, char **argv)
{
	int score = 0, hi_score = 0;
	struct map map, o_map;
	enum map_snake_state state;
	enum map_block_type dir;
	char map_str[MAX_MAP_STR_LEN+1];
	bool paused = false;
	bool should_close = false;
	int c;

	if (argc < 2 || map_parse_file(&map, argv[1]) < 0)
	{
		fprintf(stderr, "usage: viborita_ncurses <valid_map_path>\n");
		return 1;
	}

	initscr();
	nodelay(stdscr, TRUE);
	curs_set(0);
	noecho();
	map_copy(&map, &o_map);

	while (!should_close)
	{
		dir = MAP_BLOCK_INVALID;

		while ((c = getch()) != ERR) switch (c)
		{
			case 'h': dir = MAP_BLOCK_SNAKE_LEFT; break;
			case 'j': dir = MAP_BLOCK_SNAKE_DOWN; break;
			case 'k': dir = MAP_BLOCK_SNAKE_UP; break;
			case 'l': dir = MAP_BLOCK_SNAKE_RIGHT; break;
			case 'p': paused = !paused; break;
			case 'q': should_close = true; break;
		}

		if (dir != MAP_BLOCK_INVALID)
		{
			paused = false;
			map_set_snake_direction(&map, dir);
		}

		if (!paused)
		{
			map_advance(&map, &state);

			switch (state)
			{
				case MAP_SNAKE_EATING:
					map_spawn_food(&map);
					score += 1;
					if (score > hi_score)
						hi_score = score;
					break;
				case MAP_SNAKE_DEAD:
					map_copy(&o_map, &map);
					score = 0;
					break;
			}
		}

		map_stringify(&map, sizeof(map_str), map_str);
		move(0, 0);
		printw(map_str);

		if (paused)
		{
			move(map.n_rows/2, (map.n_cols-sizeof(PAUSE_MSG))/2);
			printw(PAUSE_MSG);
		}

		move(map.n_rows, 0);
		printw("Highest score: %d", hi_score);
		move(map.n_rows + 1, 0);
		printw("Score: %d", score);

		refresh();

		usleep(100000);
	}

	endwin();
	printf("Highest score: %d\n", hi_score);

	return 0;
}
