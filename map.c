#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "map.h"
#include "debug.h"

extern char *app_name;

int map_init(struct map *map, size_t n_cols, size_t n_rows)
{
	if (map == NULL ||
			n_cols == 0 || n_cols > MAX_COLS ||
			n_rows == 0 || n_rows > MAX_ROWS)
	{
		dbg_print(stderr, "%s: map_init: invalid arguments "
				"{(map = %p), (n_cols = %zu), (n_rows = %zu)}\n",
				app_name, map, n_cols, n_rows);
		return -1;
	}

	memset(map->map, 0, MAX_ROWS * MAX_COLS * sizeof(int));

	map->n_columns = n_cols;
	map->n_rows = n_rows;

	/* OK */
	return 0;
}

static size_t _map_str_count_rows(const char *map_str)
{
	size_t n_rows = 0;
	const char *walk = map_str;
	for (; *walk != '\0'; ++walk)
	{
		if (*walk == '\n')
			n_rows += 1;
	}

	if (walk != map_str && walk[-1] != '\n')
		n_rows += 1;

	return n_rows;
}

static size_t _map_str_count_cols(const char *map_str)
{
	size_t n_cols = 0;
	for (const char *walk = map_str; *walk != '\n' && *walk != '\0'; ++walk)
		n_cols += 1;
	return n_cols;
}

static enum map_block_type _map_block_type_from_char(char c)
{
	switch (c)
	{
		case ' ': return MAP_SPACE;
		case '=': return MAP_BLOCK;
		case 'o': return MAP_VIBORITA;
		case '*': return MAP_FOOD;
		default:  return MAP_SPACE;
	}
}

int map_parse(struct map *map, const char *map_str)
{
	/* count rows & columns */
	size_t n_rows = _map_str_count_rows(map_str);
	size_t n_cols = _map_str_count_cols(map_str);

	if (n_rows == 0 || n_rows > MAX_ROWS)
	{
		dbg_print(stderr, "%s: map_parse: invalid rows (rows = %zu)\n",
				app_name, n_rows);
		return -1;
	}

	if (n_cols == 0 || n_cols > MAX_COLS)
	{
		dbg_print(stderr, "%s: map_parse: invalid cols (cols = %zu)\n",
				app_name, n_cols);
		return -1;
	}

	if (map_init(map, n_cols, n_rows) < 0)
	{
		dbg_print(stderr, "%s: map_parse: could not init map\n", app_name);
		return -1;
	}

	size_t row = 0, col = 0;
	const char *walk = map_str;
	char block_char;
	enum map_block_type block_type;

	while (1) switch ((block_char = *walk++))
	{
		case '\0':
			if (col != n_cols && col != 0)
			{
				dbg_print(stderr, "%s: map_parse: parsing error at {(row=%zu, col=%zu)}\n",
						app_name, row + 1, col + 1);
				return -1;
			}
			return 0;
		case '\n':
			if (col != n_cols)
			{
				dbg_print(stderr, "%s: map_parse: parsing error at {(row=%zu, col=%zu)}\n",
						app_name, row + 1, col + 1);
				return -1;
			}
			col = 0;
			row += 1;
			break;
		case ' ': /* SPACE */
		case '=': /* BLOCK */
		case 'o': /* VIBORITA */
		case '*': /* FOOD */
			if (col >= n_cols || row >= n_rows)
			{
				dbg_print(stderr, "%s: map_parse: parsing error at {(row=%zu, col=%zu)}\n",
						app_name, row + 1, col + 1);
				return -1;
			}
			block_type = _map_block_type_from_char(block_char);
			map->map[row][col] = block_type;
			col++;
			break;
		default:
			dbg_print(stderr, "%s: map_parse: parsing error at {(row=%zu, col=%zu)}\n",
					app_name, row + 1, col + 1);
			return -1;
	}

	return 0;
}

int map_parse_file(struct map *map, const char *path)
{
	extern int dump_file_cts(const char *, size_t, char *);
	char map_str[(MAX_COLS+1) * MAX_ROWS + 1 /* NUL char */];
	if (dump_file_cts(path, sizeof(map_str), map_str) < 0)
	{
		dbg_print(stderr, "%s: map_parse_file: could not open: %s\n",
				app_name, path);
		return -1;
	}

	return map_parse(map, map_str);
}

void map_print(const struct map *map)
{

}
