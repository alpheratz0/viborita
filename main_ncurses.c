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
		return 1;

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
