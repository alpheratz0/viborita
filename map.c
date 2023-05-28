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

static size_t __map_str_count_rows(const char *map_str)
{
	size_t n_rows = 0;
	const char *walk = map_str;
	for (; *walk != '\0'; ++walk)
	{
		if (*walk == '\n')
		{
			n_rows += 1;
		}
	}

	if (walk != map_str && walk[-1] != '\n')
	{
		n_rows += 1;
	}

	return n_rows;
}

static size_t __map_str_count_cols(const char *map_str)
{
	size_t n_cols = 0;
	for (const char *walk = map_str; *walk != '\n' && *walk != '\0'; ++walk)
	{
		n_cols += 1;
	}

	return n_cols;
}

int map_parse(struct map *map, const char *map_str)
{
	if (NULL == map || NULL == map_str)
	{
		dbg_print(stderr, "%s: map_parse: invalid arguments "
				"{(map=%p), (map_str=%p)}\n",
				app_name, map, map_str);
		return -1;
	}

	/* count rows & columns */
	size_t n_rows = __map_str_count_rows(map_str);
	size_t n_cols = __map_str_count_cols(map_str);

	if (n_rows == 0 || n_rows > MAX_ROWS ||
			n_cols == 0 || n_cols > MAX_COLS)
	{
		dbg_print(stderr, "%s: map_parse: invalid map "
				"{(rows=%zu), (cols=%zu)}\n",
				app_name, n_rows, n_cols);
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
			block_type = MAP_BLOCK_TYPE_FROM_CHAR(block_char);
			if (col >= n_cols || row >= n_rows || block_type == MAP_BLOCK_INVALID)
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
	if (NULL == map || NULL == path)
	{
		dbg_print(stderr, "%s: map_parse_file: invalid arguments "
				"{(map=%p), (path=%p)}\n",
				app_name, map, path);
		return -1;
	}

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
	if (NULL == map || NULL == str || max_size == 0)
	{
		dbg_print(stderr, "%s: map_stringify: invalid arguments "
				"{(map=%p), (max_size=%zu, requested_size=%zu), (str=%p)}\n",
				app_name, map, max_size, requested_size, str);
		return -1;
	}

	char *walk = str;
	for (size_t row = 0; row < map->n_rows; ++row, *walk++ = '\n')
	{
		for (size_t col = 0; col < map->n_columns; ++col)
		{
			*walk++ = MAP_BLOCK_TYPE_TO_CHAR(map->map[row][col]);
		}
	}
	*walk = '\0';

	return 0;
}

int map_get_ref(struct map *map, size_t col, size_t row, enum map_block_type **bt)
{
	if (col >= map->n_columns ||
			row >= map->n_rows)
	{
		dbg_print(stderr, "%s: map_get_ref: invalid index "
				"{(col=%zu), (row=%zu)}\n",
				app_name, col, row);
		return -1;
	}

	*bt = &map->map[row][col];
	return 0;
}

int map_get(const struct map *map, size_t col, size_t row, enum map_block_type *bt)
{
	if (NULL == map || NULL == bt)
	{
		dbg_print(stderr, "%s: map_get: invalid arguments "
				"{(map=%p), (col=%zu), (row=%zu), (bt=%p)}\n",
				app_name, map, col, row, bt);
		return -1;
	}

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
	if (NULL == map || MAP_BLOCK_TYPE_TO_CHAR(bt) == '?')
	{
		dbg_print(stderr, "%s: set: invalid arguments "
				"{(map=%p), (col=%zu), (row=%zu), (bt=%lu)}\n",
				app_name, map, col, row, bt);
		return -1;
	}
	enum map_block_type *block_ref;
	if (map_get_ref(map, col, row, &block_ref) < 0)
	{
		dbg_print(stderr, "%s: map_set: map_get_ref failed\n", app_name);
		return -1;
	}
	*block_ref = bt;
	return 0;
}

int map_get_vibora_prev_block(struct map *map, size_t row, size_t col, size_t *prev_row, size_t *prev_col)
{
	if (col > 0)
	{
		*prev_row = row; *prev_col = col - 1;
		if (map->map[*prev_row][*prev_col] == MAP_BLOCK_VIBORITA_RIGHT)
		{
			return 0;
		}
	}
	if (col < map->n_columns - 1)
	{
		*prev_row = row; *prev_col = col + 1;
		if (map->map[*prev_row][*prev_col] == MAP_BLOCK_VIBORITA_LEFT)
		{
			return 0;
		}
	}
	if (row > 0)
	{
		*prev_row = row - 1; *prev_col = col;
		if (map->map[*prev_row][*prev_col] == MAP_BLOCK_VIBORITA_DOWN)
		{
			return 0;
		}
	}
	if (row < map->n_rows - 1)
	{
		*prev_row = row + 1; *prev_col = col;
		if (map->map[*prev_row][*prev_col] == MAP_BLOCK_VIBORITA_UP)
		{
			return 0;
		}
	}

	return -1;
}

int map_get_vibora_next_block(struct map *map, size_t row, size_t col, size_t *next_row, size_t *next_col)
{
	int tmp_next_row = (int) row,
		tmp_next_col = (int) col;

	switch (map->map[row][col])
	{
		case MAP_BLOCK_VIBORITA_UP:    tmp_next_row -= 1; break;
		case MAP_BLOCK_VIBORITA_DOWN:  tmp_next_row += 1; break;
		case MAP_BLOCK_VIBORITA_LEFT:  tmp_next_col -= 1; break;
		case MAP_BLOCK_VIBORITA_RIGHT: tmp_next_col += 1; break;
		default: return -1;
	}

	if (tmp_next_row < 0 || tmp_next_row >= map->n_rows ||
			tmp_next_col < 0 || tmp_next_col >= map->n_columns)
	{
		return -1;
	}

	*next_row = tmp_next_row;
	*next_col = tmp_next_col;

	return 0;
}

int map_is_head(struct map *map, size_t row, size_t col)
{
	enum map_block_type block = map->map[row][col];
	size_t next_row, next_col;

	if (!MAP_BLOCK_TYPE_IS_VIBORA(block))
	{
		return 0;
	}
	if (map_get_vibora_next_block(map, row, col, &next_row, &next_col) < 0 ||
			!MAP_BLOCK_TYPE_IS_VIBORA(map->map[next_row][next_col]))
	{
		return 1;
	}
	return 0;
}

int map_is_tail(struct map *map, size_t row, size_t col)
{
	enum map_block_type block = map->map[row][col];
	size_t prev_row, prev_col;

	if (!MAP_BLOCK_TYPE_IS_VIBORA(block))
	{
		return 0;
	}
	if (map_get_vibora_prev_block(map, row, col, &prev_row, &prev_col) < 0 ||
			!MAP_BLOCK_TYPE_IS_VIBORA(map->map[prev_row][prev_col]))
	{
		return 1;
	}
	return 0;
}

int map_find(struct map *map, int (*pred)(struct map *, size_t, size_t), size_t *row, size_t *col)
{
	for (size_t r = 0; r < map->n_rows; r += 1)
	{
		for (size_t c = 0; c < map->n_columns; c += 1)
		{
			if (pred(map, r, c) > 0)
			{
				*row = r;
				*col = c;
				return 1;
			}
		}
	}

	return 0;
}

int map_find_viborita_head(struct map *map, size_t *row, size_t *col)
{
	if (map_find(map, map_is_head, row, col) <= 0)
	{
		return -1;
	}

	return 0;
}

int map_find_viborita_tail(struct map *map, size_t *row, size_t *col)
{
	if (map_find(map, map_is_tail, row, col) <= 0)
	{
		return -1;
	}

	return 0;
}
