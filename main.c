#include "map.h"
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

int
main(void)
{
	struct map default_map;
	struct map pmap; // Playing map.
	enum map_viborita_state viborita_state;
	char map_str[2949];
	int c;

	if (map_parse_file(&default_map, "./maps/default") < 0)
		return 1;

	initscr();
	nodelay(stdscr, TRUE);
	noecho();

RESET:
	map_copy(&default_map, &pmap);

	do
	{
		usleep(100000);

		while ((c = getch()) != ERR) switch (c)
		{
			case 'h': map_set_viborita_direction(&pmap, MAP_BLOCK_VIBORITA_LEFT); break;
			case 'j': map_set_viborita_direction(&pmap, MAP_BLOCK_VIBORITA_DOWN); break;
			case 'k': map_set_viborita_direction(&pmap, MAP_BLOCK_VIBORITA_UP); break;
			case 'l': map_set_viborita_direction(&pmap, MAP_BLOCK_VIBORITA_RIGHT); break;
		}

		map_advance(&pmap, &viborita_state);
		map_stringify(&pmap, sizeof(map_str), map_str);
		move(0, 0);
		printw(map_str);
		refresh();
	}
	while (viborita_state != MAP_VIBORITA_DEAD);

	goto RESET;

	endwin();

	return 0;
}
