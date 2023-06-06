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

#pragma once

#include <stddef.h>

#define MAX_COLS 100
#define MAX_ROWS 100
#define MAX_MAP_STR_LEN ((MAX_COLS+1)*MAX_ROWS)

#define MAP_BLOCK_TYPE_IS_SNAKE(bt) \
	(bt == MAP_BLOCK_SNAKE_DOWN || \
		bt == MAP_BLOCK_SNAKE_LEFT || \
		bt == MAP_BLOCK_SNAKE_RIGHT || \
		bt == MAP_BLOCK_SNAKE_UP)

#define MAP_BLOCK_TYPE_FROM_CHAR(ch) \
	(ch == ' ' ? MAP_BLOCK_SPACE : \
		ch == '=' ? MAP_BLOCK_WALL : \
		ch == '*' ? MAP_BLOCK_FOOD : \
		ch == '^' ? MAP_BLOCK_SNAKE_UP : \
		ch == '<' ? MAP_BLOCK_SNAKE_LEFT : \
		ch == 'v' ? MAP_BLOCK_SNAKE_DOWN : \
		ch == '>' ? MAP_BLOCK_SNAKE_RIGHT : \
		            MAP_BLOCK_INVALID)

#define MAP_BLOCK_TYPE_TO_CHAR(bt) \
	(bt == MAP_BLOCK_SPACE ? ' ' : \
		bt == MAP_BLOCK_WALL ? '=' : \
		bt == MAP_BLOCK_FOOD ? '*' : \
		bt == MAP_BLOCK_SNAKE_UP ? '^' : \
		bt == MAP_BLOCK_SNAKE_LEFT ? '<' : \
		bt == MAP_BLOCK_SNAKE_DOWN ? 'v' : \
		bt == MAP_BLOCK_SNAKE_RIGHT ? '>' : '?')

#define MAP_FOR_EACH_BLOCK(m, row, col, block) \
	for (size_t row = 0, col = 0; row < (m)->n_rows; ++row, col = 0) \
		for ( \
			enum map_block_type block; \
			col < (m)->n_cols && (block = (m)->map[row][col]) == block; \
			++col \
		) \

enum map_block_type
{
	MAP_BLOCK_SPACE,
	MAP_BLOCK_WALL,
	MAP_BLOCK_FOOD,
	MAP_BLOCK_SNAKE_UP,
	MAP_BLOCK_SNAKE_LEFT,
	MAP_BLOCK_SNAKE_DOWN,
	MAP_BLOCK_SNAKE_RIGHT,
	MAP_BLOCK_INVALID
};

enum map_snake_state
{
	MAP_SNAKE_DEAD,
	MAP_SNAKE_IDLE,
	MAP_SNAKE_EATING
};

struct map
{
	size_t head_row, head_col;
	size_t tail_row, tail_col;
	size_t n_cols, n_rows;
	enum map_block_type map[MAX_COLS][MAX_ROWS];
	enum map_block_type dir;
};

void map_copy(const struct map *from, struct map *to);
int map_parse(struct map *map, const char *map_str);
int map_parse_file(struct map *map, const char *path);
int map_stringify(const struct map *map, size_t max_size, char *str);
int map_contains(const struct map *map, size_t row, size_t col);
int map_get(const struct map *map, size_t row, size_t col,
		enum map_block_type *bt);
int map_set(struct map *map, size_t row, size_t col, enum map_block_type bt);
int map_find_snake_prev_block(struct map *map, size_t row, size_t col,
		size_t *prev_row, size_t *prev_col);
int map_find_snake_next_block(struct map *map, size_t row, size_t col,
		size_t *next_row, size_t *next_col);
int map_is_head(struct map *map, size_t row, size_t col);
int map_is_tail(struct map *map, size_t row, size_t col);
int map_find(struct map *map, int (*pred)(struct map *, size_t, size_t),
		size_t *row, size_t *col);
int map_find_snake_head(struct map *map, size_t *row, size_t *col);
int map_find_snake_tail(struct map *map, size_t *row, size_t *col);
int map_set_snake_direction(struct map *map, enum map_block_type dir);
int map_advance(struct map *map, enum map_snake_state *snake_state);
int map_spawn_food(struct map *map);
