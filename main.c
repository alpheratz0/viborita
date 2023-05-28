#include "map.h"

char *app_name = "main.c";

int
main(void)
{
	struct map default_map;

	map_parse_file(&default_map, "./maps/default");

	return 0;
}
