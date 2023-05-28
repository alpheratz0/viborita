#include "map.h"
#include <stdio.h>

char *app_name = "main.c";

int
main(void)
{
	struct map default_map;
	char map_str[2949];

	map_parse_file(&default_map, "./maps/default");
	map_stringify(&default_map, sizeof(map_str), map_str);
	printf("%s", map_str);

	return 0;
}
