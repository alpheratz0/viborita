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
				"{(map=%p), (n_cols=%zu), (n_rows=%zu)}\n",
				app_name, map, n_cols, n_rows);
		return -1;
	}

	memset(map->map, 0, sizeof(map->map));

	map->n_columns = n_cols;
	map->n_rows = n_rows;

	/* OK */
	return 0;
}

int map_copy(const struct map *from, struct map *to)
{
	if (NULL == from || NULL == to)
	{
		dbg_print(stderr, "%s: map_copy: invalid arguments "
				"{(from=%p), (to=%p)}\n",
				app_name, from, to);
		return -1;
	}

	memcpy(to, from, sizeof(struct map));
	return 0;
}

size_t map_str_count_rows(const char *map_str)
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

size_t map_str_count_cols(const char *map_str)
{
	size_t n_cols = 0;
	for (const char *walk = map_str; *walk != '\n' && *walk != '\0'; ++walk)
		n_cols += 1;
	return n_cols;
}

enum map_block_type map_block_type_from_char(char c)
{
	switch (c)
	{
		case ' ': return MAP_BLOCK_SPACE;
		case '=': return MAP_BLOCK_WALL;
		case '*': return MAP_BLOCK_FOOD;
		case '^': return MAP_BLOCK_VIBORITA_UP;
		case '<': return MAP_BLOCK_VIBORITA_LEFT;
		case 'v': return MAP_BLOCK_VIBORITA_DOWN;
		case '>': return MAP_BLOCK_VIBORITA_RIGHT;
		default:  return MAP_BLOCK_INVALID;
	}
}

char map_block_type_to_char(enum map_block_type bt)
{
	switch (bt)
	{
		case MAP_BLOCK_SPACE:          return ' ';
		case MAP_BLOCK_WALL:           return '=';
		case MAP_BLOCK_FOOD:           return '*';
		case MAP_BLOCK_VIBORITA_UP:    return '^';
		case MAP_BLOCK_VIBORITA_LEFT:  return '<';
		case MAP_BLOCK_VIBORITA_DOWN:  return 'v';
		case MAP_BLOCK_VIBORITA_RIGHT: return '>';
		default:                       return '?';
	}
}

int map_parse(struct map *map, const char *map_str)
{
	/* count rows & columns */
	size_t n_rows = map_str_count_rows(map_str);
	size_t n_cols = map_str_count_cols(map_str);

	if (n_rows == 0 || n_rows > MAX_ROWS)
	{
		dbg_print(stderr, "%s: map_parse: invalid rows (rows=%zu)\n",
				app_name, n_rows);
		return -1;
	}

	if (n_cols == 0 || n_cols > MAX_COLS)
	{
		dbg_print(stderr, "%s: map_parse: invalid cols (cols=%zu)\n",
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
		default:
			block_type = map_block_type_from_char(block_char);
			if (col >= n_cols || row >= n_rows || block_type == MAP_INVALID)
			{
				dbg_print(stderr, "%s: map_parse: parsing error at {(row=%zu, col=%zu)}\n",
						app_name, row + 1, col + 1);
				return -1;
			}

			map->map[row][col] = block_type;
			col++;
			break;
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

int map_stringify(const struct map *map, size_t max_size, char *str)
{
	size_t requested_size = (map->n_rows * (map->n_columns + 1) + 1);
	if (max_size < requested_size)
	{
		dbg_print(stderr, "%s: map_stringify: max_size too small "
				"(max_size=%zu, requested_size=%zu)\n",
				app_name, max_size, requested_size);
		return -1;
	}

	char *walk = str;
	for (size_t row = 0; row < map->n_rows; ++row, *walk++ = '\n')
		for (size_t col = 0; col < map->n_columns; ++col)
			*walk++ = map_block_type_to_char(map->map[row][col]);
	*walk = '\0';

	return 0;
}

int map_get_ref(struct map *map, size_t col, size_t row, enum map_block_type **bt)
{
	if (col >= map->n_columns)
	{
		dbg_print(stderr, "%s: map_get_ref: invalid column index (col=%zu)",
				app_name, col);
		return -1;
	}

	if (row >= map->n_rows)
	{
		dbg_print(stderr, "%s: map_get_ref: invalid row index (row=%zu)",
				app_name, row);
		return -1;
	}

	*bt = &map->map[row][col];
	return 0;
}

int map_get(const struct map *map, size_t col, size_t row, enum map_block_type *bt)
{
	enum map_block_type *block_ref;
	if (map_get_ref((struct map *)map, col, row, &block_ref) < 0)
	{
		dbg_print(stderr, "%s: map_get: map_get_ref failed\n", app_name);
		return -1;
	}
	*bt = *block_ref;
	return 0;
}

int map_set(struct map *map, size_t col, size_t row, enum map_block_type bt)
{
	enum map_block_type *block_ref;
	if (map_get_ref(map, col, row, &block_ref) < 0)
	{
		dbg_print(stderr, "%s: map_set: map_get_ref failed\n", app_name);
		return -1;
	}
	*block_ref = bt;
	return 0;
}
