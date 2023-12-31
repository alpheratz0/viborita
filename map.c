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

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "map.h"

// Returns the numbers of rows an unparsed map has.
static size_t __map_str_count_rows(const char *map_str)
{
	size_t n_rows = 0;
	const char *walk = map_str;
	for (; *walk != '\0'; ++walk)
		if (*walk == '\n')
			n_rows += 1;
	if (walk != map_str && walk[-1] != '\n')
		n_rows += 1;
	return n_rows;
}

// Returns the numbers of columns an unparsed map has.
static size_t __map_str_count_columns(const char *map_str)
{
	size_t n_cols = 0;
	for (; *map_str != '\n' && *map_str != '\0'; ++map_str)
		n_cols += 1;
	return n_cols;
}

// Copies one map to another.
void map_copy(const struct map *from, struct map *to)
{
	memcpy(to, from, sizeof(struct map));
}

// Parses a map from a string.
int map_parse(struct map *map, const char *map_str)
{
	size_t row = 0, col = 0;
	char block_char;
	enum map_block_type block_type;

	size_t n_rows = __map_str_count_rows(map_str);
	size_t n_cols = __map_str_count_columns(map_str);

	if (n_rows == 0 || n_rows > MAX_ROWS ||
			n_cols == 0 || n_cols > MAX_COLS)
		return -1;

	memset(map->map, 0, sizeof(map->map));

	map->n_cols = n_cols;
	map->n_rows = n_rows;

	while (1) switch ((block_char = *map_str++))
	{
		case '\0':
			if (col != n_cols && col != 0 ||
				map_find_snake_head(map, &map->head_row, &map->head_col) < 0 ||
				map_find_snake_tail(map, &map->tail_row, &map->tail_col) < 0)
			{
				return -1;
			}
			map->dir = map->map[map->head_row][map->head_col];
			return 0;
		case '\n':
			if (col != n_cols)
				return -1;
			col = 0;
			row += 1;
			break;
		default:
			block_type = MAP_BLOCK_TYPE_FROM_CHAR(block_char);
			if (col >= n_cols || row >= n_rows ||
					block_type == MAP_BLOCK_INVALID)
			{
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
	// Helper fn to get file contents.
	extern int dump_file_cts(const char *, size_t, char *);

	char map_str[MAX_MAP_STR_LEN + 1 /* NUL char */];

	if (dump_file_cts(path, sizeof(map_str), map_str) < 0)
		return -1;

	return map_parse(map, map_str);
}

int map_stringify(const struct map *map, size_t max_size, char *str)
{
	size_t requested_size = (map->n_rows * (map->n_cols + 1) + 1);

	if (max_size < requested_size)
		return -1;

	for (size_t row = 0; row < map->n_rows; ++row, *str++ = '\n')
		for (size_t col = 0; col < map->n_cols; ++col)
				*str++ = MAP_BLOCK_TYPE_TO_CHAR(map->map[row][col]);

	*str = '\0';

	return 0;
}

int map_contains(const struct map *map, size_t row, size_t col)
{
	if (row >= map->n_rows ||
			col >= map->n_cols)
		return 0;
	return 1;
}

int map_get(const struct map *map, size_t row, size_t col,
		enum map_block_type *bt)
{
	if (!map_contains(map, row, col))
		return 0;
	*bt = map->map[row][col];
	return 0;
}

int map_set(struct map *map, size_t row, size_t col, enum map_block_type bt)
{
	if (!map_contains(map, row, col))
		return 0;
	map->map[row][col] = bt;
	return 0;
}

int map_find_snake_prev_block(struct map *map, size_t row, size_t col,
		size_t *prev_row, size_t *prev_col)
{
	if (col>0 && map->map[row][col-1] == MAP_BLOCK_SNAKE_RIGHT)
		*prev_row = row, *prev_col = col - 1;
	else if (col<map->n_cols-1 && map->map[row][col+1] == MAP_BLOCK_SNAKE_LEFT)
		*prev_row = row, *prev_col = col + 1;
	else if (row>0 && map->map[row-1][col] == MAP_BLOCK_SNAKE_DOWN)
		*prev_row = row - 1, *prev_col = col;
	else if (row<map->n_rows-1 && map->map[row+1][col] == MAP_BLOCK_SNAKE_UP)
		*prev_row = row + 1, *prev_col = col;
	else
		return -1;
	return 0;
}

int map_find_snake_next_block(struct map *map, size_t row, size_t col,
		size_t *next_row, size_t *next_col)
{
	int tmp_next_row = (int) row,
		tmp_next_col = (int) col;

	switch (map->map[row][col])
	{
		case MAP_BLOCK_SNAKE_UP:    tmp_next_row -= 1; break;
		case MAP_BLOCK_SNAKE_DOWN:  tmp_next_row += 1; break;
		case MAP_BLOCK_SNAKE_LEFT:  tmp_next_col -= 1; break;
		case MAP_BLOCK_SNAKE_RIGHT: tmp_next_col += 1; break;
		default: return -1;
	}

	if (tmp_next_row < 0 || tmp_next_row >= map->n_rows ||
			tmp_next_col < 0 || tmp_next_col >= map->n_cols)
		return -1;

	*next_row = tmp_next_row;
	*next_col = tmp_next_col;

	return 0;
}

int map_is_head(struct map *map, size_t row, size_t col)
{
	enum map_block_type block = map->map[row][col];
	size_t next_row, next_col;

	if (!MAP_BLOCK_TYPE_IS_SNAKE(block))
		return 0;

	if (map_find_snake_next_block(map, row, col, &next_row, &next_col) < 0 ||
			!MAP_BLOCK_TYPE_IS_SNAKE(map->map[next_row][next_col]))
		return 1;

	return 0;
}

int map_is_tail(struct map *map, size_t row, size_t col)
{
	enum map_block_type block = map->map[row][col];
	size_t prev_row, prev_col;

	if (!MAP_BLOCK_TYPE_IS_SNAKE(block))
		return 0;

	if (map_find_snake_prev_block(map, row, col, &prev_row, &prev_col) < 0 ||
			!MAP_BLOCK_TYPE_IS_SNAKE(map->map[prev_row][prev_col]))
		return 1;

	return 0;
}

int map_find(struct map *map, int (*pred)(struct map *, size_t, size_t),
		size_t *row, size_t *col)
{
	for (size_t r = 0; r < map->n_rows; r += 1)
	{
		for (size_t c = 0; c < map->n_cols; c += 1)
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

int map_find_snake_head(struct map *map, size_t *row, size_t *col)
{
	if (map_find(map, map_is_head, row, col) <= 0)
		return -1;
	return 0;
}

int map_find_snake_tail(struct map *map, size_t *row, size_t *col)
{
	if (map_find(map, map_is_tail, row, col) <= 0)
		return -1;
	return 0;
}

int map_set_snake_direction(struct map *map, enum map_block_type dir)
{
	size_t next_head_row, next_head_col;
	size_t head_row, head_col;
	size_t prev_head_row, prev_head_col;
	enum map_block_type head_block;

	head_row = map->head_row;
	head_col = map->head_col;
	head_block = map->map[head_row][head_col];

	map_find_snake_prev_block(map, head_row, head_col, &prev_head_row,
			&prev_head_col);
	map->map[head_row][head_col] = dir;
	map_find_snake_next_block(map, head_row, head_col, &next_head_row,
			&next_head_col);

	if (prev_head_row == next_head_row &&
			prev_head_col == next_head_col)
	{
		map->map[head_row][head_col] = head_block;
		return -1;
	}

	return 0;
}

int map_advance(struct map *map, enum map_snake_state *snake_state)
{
	size_t head_row, head_col;
	size_t head_next_row, head_next_col;
	size_t tail_row, tail_col;

	head_row = map->head_row;
	head_col = map->head_col;
	tail_row = map->tail_row;
	tail_col = map->tail_col;

	// Check if the snake goes outside the map.
	if (map_find_snake_next_block(map, head_row, head_col,
				&head_next_row, &head_next_col) < 0)
	{
		*snake_state = MAP_SNAKE_DEAD;
		return 0;
	}

	switch (map->map[head_next_row][head_next_col])
	{
		case MAP_BLOCK_SPACE:
			map->map[head_next_row][head_next_col] =
				map->map[head_row][head_col];
			*snake_state = MAP_SNAKE_IDLE;
			break;
		case MAP_BLOCK_FOOD:
			map->map[head_next_row][head_next_col] =
				map->map[head_row][head_col];
			*snake_state = MAP_SNAKE_EATING;
			break;
		default:
			*snake_state = MAP_SNAKE_DEAD;
			return 0;
	}

	if (*snake_state != MAP_SNAKE_EATING)
	{
		map_find_snake_next_block(map, tail_row, tail_col, &map->tail_row,
				&map->tail_col);
		map->map[tail_row][tail_col] = MAP_BLOCK_SPACE;
	}

	map->head_row = head_next_row;
	map->head_col = head_next_col;

	return 0;
}

int map_spawn_food(struct map *map)
{
	size_t n_space_blocks = 0;
	MAP_FOR_EACH_BLOCK(map, row, col, block)
		if (block == MAP_BLOCK_SPACE)
			n_space_blocks += 1;
	if (n_space_blocks == 0)
		return -1;
	size_t n_food_block = rand() % n_space_blocks;
	MAP_FOR_EACH_BLOCK(map, row, col, block)
	{
		if (block != MAP_BLOCK_SPACE)
			continue;
		if (n_food_block == 0)
			map->map[row][col] = MAP_BLOCK_FOOD;
		n_food_block -= 1;
	}
	return 0;
}
