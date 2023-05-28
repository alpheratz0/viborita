#include "map.h"
#include <stdio.h>
#include <unistd.h>

char *app_name = "main.c";

int
main(void)
{
	struct map default_map;
	enum map_viborita_state viborita_state;
	char map_str[2949];

	map_parse_file(&default_map, "./maps/default");

	do
	{
		map_advance(&default_map, &viborita_state);
		map_stringify(&default_map, sizeof(map_str), map_str);
		printf("%s", map_str);
		sleep(1);
	}
	while (viborita_state != MAP_VIBORITA_DEAD);

	return 0;
}
